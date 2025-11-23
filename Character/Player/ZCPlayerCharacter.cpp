// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/ZCPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "MotionWarpingComponent.h"
#include "Component/LockOn/ZCLockSpringArmComponent.h"
#include "Component/Climb/ZCCharacterMovementComponent.h"
#include "Component/State/Player/ZCPlayerStateComponent.h"
#include "Player/ZCPlayerController.h"
#include "Actor/Item/Weapon/ZCWeaponActor.h"

#include "Component/LockOn/ZCLockTargetComponent.h"

#include "Game/Subsystem/ZCItemGISubsystem.h"
#include "World/Subsystem/ZCWorldSubsystem.h"

#include "Utils/Team/ZCTeam.h"

#include "Actor/ZCActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCPlayerCharacter)

namespace PlayerInputType
{
	static const FName Attack = TEXT("Attack");
	static const FName Guard = TEXT("Parry");
}

AZCPlayerCharacter::AZCPlayerCharacter(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer.SetDefaultSubobjectClass<UZCCharacterMovementComponent>(AZCCharacter::CharacterMovementComponentName)
		.SetDefaultSubobjectClass<UZCPlayerStateComponent>(AZCCharacter::StateComponentName))
{
	// Set this character to call Tick() every frame.

	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->SetCollisionProfileName(Zelda::Profile::Player);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanFly = true;

	GetCharacterMovement()->SetCrouchedHalfHeight(48.0f);	// 캡슐의 절반
	GetCharacterMovement()->SetWalkableFloorAngle(60.0f);

	CameraBoom = CreateDefaultSubobject<UZCLockSpringArmComponent>(TEXT("CameraArm"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, UZCLockSpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GlideMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GlideMesh"));
	GlideMeshComponent->SetupAttachment(GetMesh(), BoneSocket::Glide::Holster);
	GlideMeshComponent->SetRelativeScale3D(FVector(0.1f));

	HighlightArea = CreateDefaultSubobject<USphereComponent>(TEXT("HighlightArea"));
	HighlightArea->InitSphereRadius(1000.0f);
	HighlightArea->SetupAttachment(RootComponent);
	HighlightArea->SetCollisionProfileName(Zelda::Profile::HighlightArea);
	HighlightArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	HighlightArea->OnComponentBeginOverlap.AddDynamic(this, &AZCPlayerCharacter::OnEnterHighlightRange);
	HighlightArea->OnComponentEndOverlap.AddDynamic(this, &AZCPlayerCharacter::OnExitHighlightRange);

	ZCCharacterMovementComponent = ExactCast<UZCCharacterMovementComponent>(GetCharacterMovement());
	ZCPlayerStateComponent = ExactCast<UZCPlayerStateComponent>(GetStateComponent());

	TeamID = Zelda::TeamID::Player;
}

void AZCPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	ZCPlayerController = CastChecked<AZCPlayerController>(GetController());
}

void AZCPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AZCPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 이동 액션
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AZCPlayerCharacter::Move);
		
		// 카메라 액션
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AZCPlayerCharacter::Look);

		// 점프 액션
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AZCPlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AZCPlayerCharacter::StopJumping);

		// 공격 액션
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AZCPlayerCharacter::StartAttack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Canceled, this, &AZCPlayerCharacter::NormalAttack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AZCPlayerCharacter::AttackCharging);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AZCPlayerCharacter::HoldAttack);

		// 패리 액션
		EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Triggered, this, &AZCPlayerCharacter::StartParry);

		// 달리기 액션
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AZCPlayerCharacter::StartRunning);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered, this, &AZCPlayerCharacter::Running);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AZCPlayerCharacter::StopRunning);

		// 주목 액션
		EnhancedInputComponent->BindAction(AttentionAction, ETriggerEvent::Started, this, &AZCPlayerCharacter::StartAttention);
		EnhancedInputComponent->BindAction(AttentionAction, ETriggerEvent::Completed, this, &AZCPlayerCharacter::StopAttention);

		// 웅크리기 액션
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AZCPlayerCharacter::ToggleCrouch);
	}
}

void AZCPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (bIsCrouched) UnCrouch();
}

void AZCPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Climbing();
}

void AZCPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ZCCharacterMovementComponent->OnMovementChangeDelegate.AddDynamic(this, &AZCPlayerCharacter::OnMovementChange);
	ZCCharacterMovementComponent->OnCanClimbDownCliffDelegate.BindUObject(this, &AZCPlayerCharacter::OnCanClimbDownCliff);
}

void AZCPlayerCharacter::CanGotoNextSection(const FName MontageName, const FName NextSectionName)
{
	if (MontageName == PlayerInputType::Attack)
	{
		// 연속공격 마지막 섹션이면 종료
		if (NextSectionName == TEXT("End"))
		{
			FinishAttack();
		}
		else
		{
			if (ConsumeBufferInput(PlayerInputType::Attack))
			{
				bAttackInputLock = false;

				++CurrentAttackComboCount;

				GotoNextMontageSection(NextSectionName);
			}
			else
			{
				FinishAttack();
			}
		}
	}
}

void AZCPlayerCharacter::OnMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode)
{
	static ECustomMovementMode PreviouseMovementMode = ECustomMovementMode::CMOVE_MAX;

	if (NewCustomMovementMode == ECustomMovementMode::CMOVE_MAX)
	{
		switch (PreviouseMovementMode)
		{
		case CMOVE_Climbing:
			break;
		case CMOVE_Gliding:
			GlidingCancel();
			break;
		case CMOVE_MAX:
			break;
		default:
			break;
		}
	}
	else
	{
		PreviouseMovementMode = NewCustomMovementMode;

		switch (NewCustomMovementMode)
		{
		case CMOVE_Climbing:
		case CMOVE_Gliding:
			UnEquipWeapon();
			UnEquipShield();
			break;
		case CMOVE_MAX:
			break;
		default:
			break;
		}
	}
}

void AZCPlayerCharacter::OnCanClimbDownCliff(const bool bDownCliff)
{
	if (bDownCliff)
	{
		ZCPlayerController->AddInteractionEvent(EFaceButtonEvent::ClimbDown);
		bCanClimbDownCliff = true;
	}
	else
	{
		ZCPlayerController->RemoveInteractionEvent(EFaceButtonEvent::ClimbDown);
		bCanClimbDownCliff = false;
	}
}

void AZCPlayerCharacter::OnEnterHighlightRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AZCActor* Item = Cast<AZCActor>(OtherActor))
	{
		Item->RequestOverlayState(EOverlayState::ItemHighlight);
	}
}

void AZCPlayerCharacter::OnExitHighlightRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AZCActor* Item = Cast<AZCActor>(OtherActor))
	{
		Item->ReleaseOverlayState(EOverlayState::ItemHighlight);
	}
}

void AZCPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MoveAxisVector = Value.Get<FVector2D>();

	if (Controller == nullptr || MoveAxisVector.IsNearlyZero())
	{
		return;
	}

	FVector DirectionForward, DirectionRight;

	if (ZCCharacterMovementComponent->IsClimbing())
	{
		// 클라이밍 중일 때 방향 계산
		const FVector SurfaceNormal = ZCCharacterMovementComponent->GetClimbSurfaceNormal();

		// 벽에 수직인 오르내림 방향
		DirectionForward = FVector::CrossProduct(SurfaceNormal, -GetActorRightVector());

		// 벽에 평행한 좌우 이동 방향
		DirectionRight = FVector::CrossProduct(SurfaceNormal, GetActorUpVector());
	}
	else
	{
		// 일반적인 방향 계산
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		DirectionForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		DirectionRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}

	AddMovementInput(DirectionForward, MoveAxisVector.Y);
	AddMovementInput(DirectionRight, MoveAxisVector.X);

	// 임시 코드
	MakeNoise(1.0f, this, GetActorLocation());
}

void AZCPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AZCPlayerCharacter::Jump()
{
	ZCPlayerController->RemoveInteractionEvent(EFaceButtonEvent::ClimbDown);
	bCanClimbDownCliff = false;

	EZCDirection CurrentDirection = GetInputDirection();
	if (bIsCrouched)
	{
		
		if (CurrentDirection == EZCDirection::None)
		{
			UnCrouch();
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]() { Super::Jump(); }));
		}
		else
		{
			switch (CurrentDirection)
			{
			case EZCDirection::Forward:
			case EZCDirection::ForwardLeft:
			case EZCDirection::ForwardRight:
				PlayAnimMontage(CrouchEvasion[EZCDirection::Forward]);
				break;

			case EZCDirection::Backward:
			case EZCDirection::BackwardLeft:
			case EZCDirection::BackwardRight:
				PlayAnimMontage(CrouchEvasion[EZCDirection::Backward]);
				break;

			case EZCDirection::Left:
				PlayAnimMontage(CrouchEvasion[EZCDirection::Left]);
				break;
			case EZCDirection::Right:
				PlayAnimMontage(CrouchEvasion[EZCDirection::Right]);
				break;

			default:
				break;
			}
		}
	}

	if (ZCCharacterMovementComponent->IsMovingOnGround())
	{
		if (!bIsAttentionTriggered)
		{
			Super::Jump(); // 땅 위면 점프
		}
		else if (CurrentDirection != EZCDirection::None)
		{
			switch (CurrentDirection)
			{
			case EZCDirection::Backward:
			case EZCDirection::BackwardLeft:
			case EZCDirection::BackwardRight:
				PlayAnimMontage(StandEvasion[EZCDirection::Backward]);
				break;

			case EZCDirection::Left:
				PlayAnimMontage(StandEvasion[EZCDirection::Left]);
				break;
			case EZCDirection::Right:
				PlayAnimMontage(StandEvasion[EZCDirection::Right]);
				break;

			default:
				Super::Jump();
				break;
			}
		}
	}
	else if (ZCCharacterMovementComponent->IsClimbing())
	{
		ZCCharacterMovementComponent->TryClimbDashing(); // 등반 중이면 대쉬
	}
	else
	{
		// 공중이면 글라이딩
		UnEquipWeapon();
		UnEquipShield();
		Gliding();
	}
}

void AZCPlayerCharacter::StopJumping()
{
	Super::StopJumping();
}

void AZCPlayerCharacter::StartAttack()
{
	if(!bIsEquipWeapon) DrawWeapon();
	if (!bIsEquipShield) EquipShield();
}

void AZCPlayerCharacter::NormalAttack()
{
	if (!bIsEquipWeapon) return;

	if (!bAttackInputLock)
	{
		if (CurrentAttackComboCount == 0)
		{
			CurrentAttackComboCount = 1;

			CurrentWeapon->SetAttackInfo(EAttackType::Normal, 0);

			if (CameraBoom->IsLocked())
			{
				MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(MotionWarping::WarpTarget::AttackTarget, CameraBoom->GetCurrentTarget()->GetComponentLocation());
			}
			else
			{
				MotionWarpingComponent->RemoveAllWarpTargets();
			}

			PlayAnimMontage(AttackMontages[CurrentWeapon->GetWeaponType()]);
		}
		else
		{
			bAttackInputLock = true;
			BufferInput(PlayerInputType::Attack);
		}
	}
}

void AZCPlayerCharacter::AttackCharging()
{
	if (!bIsEquipWeapon) return;

	if (bChargingAttack == false)
	{
		// 모으기 시작
		bChargingAttack = true;
		AttackChargingGauge = 0;
	}
	else
	{
		// 이미 모으기 중이면 모으기 게이지 증가
		++AttackChargingGauge;

		CurrentWeapon->SetAttackInfo(EAttackType::Charging, AttackChargingGauge);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			UAnimMontage* ChargingMontage = AttackChargingMontages[CurrentWeapon->GetWeaponType()];
			if (!AnimInstance->Montage_IsPlaying(ChargingMontage))
			{
				AnimInstance->Montage_Play(ChargingMontage, 1.0f);
			}
		}
	}
}

void AZCPlayerCharacter::HoldAttack()
{
	if (!bIsEquipWeapon) return;

	bChargingAttack = false;

	CurrentWeapon->SetAttackInfo(EAttackType::AttackCharge, AttackChargingGauge);

	PlayAnimMontage(ChargeAttackMontages[CurrentWeapon->GetWeaponType()]);
	FinishAttack();
}

void AZCPlayerCharacter::FinishAttack()
{
	if (!bIsEquipWeapon) return;

	bAttackInputLock = false;
	CurrentAttackComboCount = 0;
	AttackChargingGauge = 0;
}

void AZCPlayerCharacter::StartParry()
{
	if (bIsAttentionTriggered && CurrentShield != nullptr && bIsEquipShield)
	{
		// 무기 패링
		PlayAnimMontage(ParryMontage);
	}
}

void AZCPlayerCharacter::StartRunning()
{
	if (ZCCharacterMovementComponent->IsGliding()) GlidingCancel();

	if (ZCCharacterMovementComponent->IsClimbing()) ClimbingCancel();

	if (bIsCrouched) UnCrouch();

	if (bIsAttentionTriggered) StopAttention();

	SheathWeapon();
	UnEquipShield();

	ZCCharacterMovementComponent->SetWantsToRun(true);
}

void AZCPlayerCharacter::Running()
{
	if (ZCCharacterMovementComponent->IsFalling()) return;

	if (GetInputDirection() == EZCDirection::None) return;
}

void AZCPlayerCharacter::StopRunning()
{
	ZCCharacterMovementComponent->SetWantsToRun(false);
}

void AZCPlayerCharacter::StartAttention()
{
	bIsAttentionTriggered = true;

	Restore_bOrientRotationToMovement = GetCharacterMovement()->bOrientRotationToMovement;
	Restore_bUseControllerDesiredRotation = GetCharacterMovement()->bUseControllerDesiredRotation;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	CameraBoom->ToggleLock();

	if (bIsEquipShield)
	{
		OnGuardStateChanged.Broadcast(true);
	}

	ZCCharacterMovementComponent->SetAttentionState(true);
}

void AZCPlayerCharacter::StopAttention()
{
	bIsAttentionTriggered = false;

	CameraBoom->BreakLock();

	GetCharacterMovement()->bOrientRotationToMovement = Restore_bOrientRotationToMovement;
	GetCharacterMovement()->bUseControllerDesiredRotation = Restore_bUseControllerDesiredRotation;

	if (bIsEquipShield)
	{
		OnGuardStateChanged.Broadcast(false);
	}

	ZCCharacterMovementComponent->SetAttentionState(false);
}

void AZCPlayerCharacter::ToggleCrouch()
{
	bIsCrouched ? UnCrouch() : Crouch();
}

void AZCPlayerCharacter::ClimbDownCliff()
{
	// 절벽에서 내려가기
	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(MotionWarping::WarpTarget::ClimbDownTarget, ZCCharacterMovementComponent->GetClimbClimbDownSurface());
	ZCCharacterMovementComponent->TryClimbDownCliff();
}

/*==================입력 버퍼===================*/
bool AZCPlayerCharacter::ConsumeBufferInput(const FName InputName)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (InputBufferMap.Contains(InputName))
	{
		const float Time = InputBufferMap[InputName];
		const float Delta = CurrentTime - Time;

		// 오차 허용
		if (Delta <= InputBufferDuration + KINDA_SMALL_NUMBER)
		{
			InputBufferMap.Remove(InputName);
			return true;
		}

		InputBufferMap.Remove(InputName); // 만료된 것도 제거
	}
	return false;
}

/*==================전투 로직===================*/
void AZCPlayerCharacter::DrawWeapon()
{
	// 무기가 존재하고 장착중이 아니고 무기 장착 애니메이션이 재생중이 아닐 때
	if (CurrentWeapon && !bIsEquipWeapon && !bIsPlayingEquipMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		bIsPlayingEquipMontage = true;

		FOnMontageEnded OnMontageEnded;
		OnMontageEnded.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) {
			bIsEquipWeapon = true;
			bIsPlayingEquipMontage = false;
			OnWeaponTypeChanged.Broadcast(CurrentWeapon->GetWeaponType());
			});

		AnimInstance->Montage_Play(DrawWeaponMontage, 1.0f);
		AnimInstance->Montage_SetEndDelegate(OnMontageEnded, DrawWeaponMontage);
	}
}

void AZCPlayerCharacter::SheathWeapon()
{
	// 무기가 존재하고 장착중이고 무기 수납 애니메이션이 재생중이 아닐 때
	if (CurrentWeapon && bIsEquipWeapon && !bIsPlayingSheathMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		bIsPlayingSheathMontage = true;

		FOnMontageEnded OnMontageEnded;
		OnMontageEnded.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) {
			bIsEquipWeapon = false;
			bIsPlayingSheathMontage = false;
			OnWeaponTypeChanged.Broadcast(EWeaponType::None);
			UnEquipWeapon();
			});

		AnimInstance->Montage_Play(SheathWeaponMontage, 1.0f);
		AnimInstance->Montage_SetEndDelegate(OnMontageEnded, SheathWeaponMontage);
	}
}

/*==================클라이밍 로직===================*/
void AZCPlayerCharacter::Climbing()
{
	// 땅 위에서는 0.5초 후 시도
	if (ZCCharacterMovementComponent->IsMovingOnGround() && ZCCharacterMovementComponent->CanStartClimbing())
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(ClimbTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(ClimbTimerHandle, [this]()
				{
					ZCCharacterMovementComponent->TryClimbing();
				}, 0.5f, false);
		}
		return;
	}

	// 낙하 중에 암벽 등반이 가능하면
	if (ZCCharacterMovementComponent->IsFalling() || ZCCharacterMovementComponent->IsGliding())
	{
		ZCCharacterMovementComponent->TryClimbing();
	}
}

void AZCPlayerCharacter::ClimbingCancel()
{
	ZCCharacterMovementComponent->CancelClimbing();
}

/*=================글라이딩 로직==================*/
void AZCPlayerCharacter::Gliding()
{
	if (ZCCharacterMovementComponent->CanStartGliding())
	{
		ZCCharacterMovementComponent->TryGliding();

		GlideMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, BoneSocket::Glide::GlideGrip);
		GlideMeshComponent->SetRelativeScale3D(FVector(0.5f));
	}
}

void AZCPlayerCharacter::GlidingCancel()
{
	ZCCharacterMovementComponent->CancelGliding();
	GlideMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, BoneSocket::Glide::Holster);
	GlideMeshComponent->SetRelativeScale3D(FVector(0.1f));
}

/*==================유틸 함수들===================*/
EZCDirection AZCPlayerCharacter::GetInputDirection() const
{
	FVector Input = GetCharacterMovement()->GetLastInputVector();
	if (Input.IsNearlyZero())
		return EZCDirection::None;

	Input.Z = 0.f; // 수직 방향 제거
	Input.Normalize();

	const FVector Forward = GetActorForwardVector();
	const FVector Right = GetActorRightVector();

	const float ForwardDot = FVector::DotProduct(Forward, Input);
	const float RightDot = FVector::DotProduct(Right, Input);

	// 각 방향에 대한 사분면 분류
	if (ForwardDot > 0.382683f) // cos(67.5도)
	{
		if (RightDot > 0.382683f)
			return EZCDirection::ForwardRight;
		else if (RightDot < -0.382683f)
			return EZCDirection::ForwardLeft;
		else
			return EZCDirection::Forward;
	}
	else if (ForwardDot < -0.382683f)
	{
		if (RightDot > 0.382683f)
			return EZCDirection::BackwardRight;
		else if (RightDot < -0.382683f)
			return EZCDirection::BackwardLeft;
		else
			return EZCDirection::Backward;
	}
	else
	{
		if (RightDot > 0)
			return EZCDirection::Right;
		else
			return EZCDirection::Left;
	}
}

FTransform AZCPlayerCharacter::GetGlideLeftHandIK_Implementation() const
{
	if (ZCCharacterMovementComponent->IsGliding())
	{
		return GlideMeshComponent->GetSocketTransform(BoneSocket::Glide::LeftHandIK);
	}
	return FTransform::Identity;
}

FTransform AZCPlayerCharacter::GetGlideRightHandIK_Implementation() const
{
	if (ZCCharacterMovementComponent->IsGliding())
	{
		return GlideMeshComponent->GetSocketTransform(BoneSocket::Glide::RightHandIK);
	}
	return FTransform::Identity;
}
