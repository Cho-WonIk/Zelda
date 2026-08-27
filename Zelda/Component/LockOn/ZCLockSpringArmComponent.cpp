// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LockOn/ZCLockSpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "ZCLockOnTrace.h"
#include "Component/LockOn/ZCLockTargetComponent.h"

#include "Development/ZCDebug.h"

#if !UE_BUILD_SHIPPING
namespace Zelda::Debug::LockOn
{
	static bool bDrawDebugVar = false;

	static FAutoConsoleVariableRef CVar_DrawDebug(
		Zelda::Debug::LockOn::DrawDebug,
		bDrawDebugVar,
		TEXT("LockOn 시스템 내 감지된 타겟과 감지 가능 범위 시각화"),
		ECVF_Default
	);
}
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCLockSpringArmComponent)

UZCLockSpringArmComponent::UZCLockSpringArmComponent()
{
	TargetArmLength = 300.0f;
	bUsePawnControlRotation = true;
	bEnableCameraLag = true;
	bEnableCameraRotationLag = true;
	CameraLagSpeed = 10.0f;
	CameraRotationLagSpeed = 15.0f;
	CameraLagMaxDistance = 100.0f;

	bDisableRotation = false;

	MaxTargetLockDistance = 750.0f;
	MaxTargetLockBreakDistance = 1500.0f;
	bAutoLockNewTargets = true;

	TargetSwitchMouseDeltaThreshold = 3.0f;
	TargetSwitchGamepadDeltaThreshold = 0.7f;
	TargetSwitchMinDelaySeconds = 0.5f;

	CharacterOrientationMode = ECharacterOrientationMode::OrientToTarget;

	LockOnRotationRate = 10.0f;

	DefaultWidgetDrawSize = FIntPoint(100, 100);
	bDefaultWidgetDrawAtDesiredSize = false;
}

void UZCLockSpringArmComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayer = Cast<ACharacter>(GetOwner());

	DefaultSocketOffset = SocketOffset;
	TargetSocketOffset = SocketOffset;
}

void UZCLockSpringArmComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSocketOffset(DeltaTime);

	if (!OwningPlayer) return;

	if (IsLocked())
	{

#if !UE_BUILD_SHIPPING
		if (Zelda::Debug::LockOn::bDrawDebugVar)
		{
			DrawDebugSphere(GetWorld(), CurrentTarget->GetComponentLocation(), 20.0f, 12, FColor::Red);
		}
#endif

		if (!bDisableRotation)
		{
			UCharacterMovementComponent* CharMovement = OwningPlayer->GetCharacterMovement();
			if (CharacterOrientationMode == ECharacterOrientationMode::OrientToTarget)
			{
				CharMovement->bOrientRotationToMovement = false;
				CharMovement->bUseControllerDesiredRotation = true;
			}
			else if (CharacterOrientationMode == ECharacterOrientationMode::OrientToMovement)
			{
				CharMovement->bOrientRotationToMovement = true;
				CharMovement->bUseControllerDesiredRotation = false;
			}

			FRotator TargetRotation = FindDesiredRotation(DeltaTime);
			FRotator CurrentContorlRot = OwningPlayer->GetControlRotation();
			FRotator NewControlRot = FMath::RInterpTo(CurrentContorlRot, TargetRotation, DeltaTime, LockOnRotationRate);
			OwningPlayer->GetController()->SetControlRotation(NewControlRot);
		}

		FVector ToTarget = CurrentTarget->GetComponentLocation() - GetComponentLocation();
		if (ToTarget.Size() > (MaxTargetLockBreakDistance + CurrentTarget->GetScaledSphereRadius()))
			BreakLockAndFindAnother();

		if (CurrentTarget && CurrentTarget->IsDisabled())
			BreakLockAndFindAnother();

		if (CurrentTarget)
		{
			FVector ToTargetNorm = (CurrentTarget->GetComponentLocation() - GetComponentLocation()).GetSafeNormal();
			float VerticalDot = FVector::DotProduct(ToTargetNorm, GetUpVector());
			if (FMath::Abs(VerticalDot) > 0.975f)
				BreakLockAndFindAnother();
		}
	}
	else
	{
		if (ShouldAlignCameraToForward)
		{
			if (bUsePawnControlRotation && !bDisableRotation)
			{
				float AlignSpeed = 20.0f;
				FRotator CameraRot = OwningPlayer->GetControlRotation();
				FRotator NesCameraRot = FMath::RInterpTo(CameraRot, ForwardAlignRot, DeltaTime, AlignSpeed);
				NesCameraRot.Pitch = CameraRot.Pitch;
				OwningPlayer->GetController()->SetControlRotation(NesCameraRot);
				if (FMath::Abs(NesCameraRot.Yaw - ForwardAlignRot.Yaw) < 0.2f)
					ShouldAlignCameraToForward = false;
			}
			else
			{
				ShouldAlignCameraToForward = false;
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (Zelda::Debug::LockOn::bDrawDebugVar)
	{
		if (IsLocked())
		{
			DrawDebugSphere(GetWorld(), GetComponentLocation(), MaxTargetLockBreakDistance, 32, FColor::Yellow);
			for (UZCLockTargetComponent* Target : GetTargetComponents(GetComponentLocation(), MaxTargetLockBreakDistance))
				DrawDebugLine(GetWorld(), GetComponentLocation(), Target->GetComponentLocation(), FColor::Green);
		}
		else
		{
			DrawDebugSphere(GetWorld(), GetComponentLocation(), MaxTargetLockDistance, 32, FColor::White);
			for (UZCLockTargetComponent* Target : GetTargetComponents(GetComponentLocation(), MaxTargetLockDistance))
				DrawDebugLine(GetWorld(), GetComponentLocation(), Target->GetComponentLocation(), FColor::Green);
		}
	}
#endif
}

FRotator UZCLockSpringArmComponent::FindDesiredRotation(float DeltaTime)
{
	// 기본 타겟 방향
	FVector ToTarget = CurrentTarget->GetComponentLocation() - GetComponentLocation();
	FRotator DesiredRotation = ToTarget.GetSafeNormal().Rotation();

	if (CameraAdjust == ECollisionChannel::ECC_MAX) return DesiredRotation;

	FVector Start = MimickCameraPositionFromControlRotation(DesiredRotation, DeltaTime);
	FVector End = CurrentTarget->GetComponentLocation();

	// 장애물 검사
	FHitResult HitResult;
	bool Hit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, CameraAdjust);
	if (Hit)
	{
		auto CameraManager = GetPlayerCameraManager();
		float PitchIncrement = -5.0f;
		// 카메라 각도 조절
		while (Hit)
		{
			if (DesiredRotation.Pitch <= CameraManager->ViewPitchMin)
				break;

			DesiredRotation.Pitch += PitchIncrement;
			Start = MimickCameraPositionFromControlRotation(DesiredRotation, DeltaTime);
			Hit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, CameraAdjust);
		}
	}

	return DesiredRotation;
}

FVector UZCLockSpringArmComponent::MimickCameraPositionFromControlRotation(FRotator Rotation, float DeltaTime)
{
	FVector ArmOrigin = GetComponentLocation() + TargetOffset;
	FVector DesiredCameraLoc = ArmOrigin;

	DesiredCameraLoc -= Rotation.Vector() * TargetArmLength;
	DesiredCameraLoc += FRotationMatrix(Rotation).TransformVector(SocketOffset);

	FVector ResultLoc;
	if (bDoCollisionTest && (TargetArmLength != 0.0f))
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
		FHitResult Result;
		GetWorld()->SweepSingleByChannel(Result, ArmOrigin, DesiredCameraLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);

		ResultLoc = BlendLocations(DesiredCameraLoc, Result.Location, Result.bBlockingHit, DeltaTime);
	}
	else
	{
		ResultLoc = DesiredCameraLoc;
	}

	return ResultLoc;
}

bool UZCLockSpringArmComponent::UpdateLookExt(FVector2D LookAxisVector, float TargetSwitchThreshold)
{
	if (ShouldAlignCameraToForward) return true;

	if (!IsLocked()) return false;

	float TimesinceLastTargetSwitch = GetWorld()->GetTimeSeconds() - LastTargetSwitchTime;

	float AbsX = FMath::Abs(LookAxisVector.X);
	float AbsY = FMath::Abs(LookAxisVector.Y);

	if (AbsX < 0.2f && AbsY < 0.2f)
		TimesinceLastTargetSwitch = TargetSwitchMinDelaySeconds;

	if (TimesinceLastTargetSwitch >= TargetSwitchMinDelaySeconds)
	{
		if ((AbsX > AbsY) && (AbsX > TargetSwitchThreshold))
		{
			ETargetSwitchDirection Dir = (LookAxisVector.X > 0.0f) ? ETargetSwitchDirection::Right : ETargetSwitchDirection::Left;
			SwitchTarget(Dir);
			LastTargetSwitchTime = GetWorld()->GetTimeSeconds();
		}
		else if ((AbsY > AbsX) && (AbsY > TargetSwitchThreshold))
		{
			ETargetSwitchDirection Dir = (LookAxisVector.Y > 0.0f) ? ETargetSwitchDirection::Up : ETargetSwitchDirection::Down;
			SwitchTarget(Dir);
			LastTargetSwitchTime = GetWorld()->GetTimeSeconds();
		}
	}

	return true;
}

void UZCLockSpringArmComponent::BreakLockAndFindAnother()
{
	BreakLock();

	if (bAutoLockNewTargets)
	{
		if (UZCLockTargetComponent* Target = FindBestTarget())
		{
			LockTarget(Target);
		}
	}
}

UZCLockTargetComponent* UZCLockSpringArmComponent::FindBestTarget()
{
	TArray<UZCLockTargetComponent*> TargetCandidates = GetTargetComponents(GetComponentLocation(), MaxTargetLockDistance);

	if (TargetCandidates.Num() == 0) return nullptr;

	UZCLockTargetComponent* BestTarget = nullptr;
	FVector PlayerForward = GetForwardVector();
	FVector PlayerLocation = GetComponentLocation();

	float BestDist = MAX_FLT;
	for (UZCLockTargetComponent* Candidate : TargetCandidates)
	{
		FVector PlayerToCandidateVector = Candidate->GetComponentLocation() - PlayerLocation;
		FVector PlayerToCandidate = PlayerToCandidateVector.GetSafeNormal();

		float Dot = FVector::DotProduct(PlayerToCandidate, PlayerForward);
		float Dist = PlayerToCandidateVector.SizeSquared();

		if ((Dist < BestDist) && (Dot >= 0.0f))
		{
			BestDist = Dist;
			BestTarget = Candidate;
		}
	}

	if (!BestTarget)
	{
		float BestDot = -MAX_FLT;
		for (UZCLockTargetComponent* Candidate : TargetCandidates)
		{
			FVector PlayerToCandidate = (Candidate->GetComponentLocation() - PlayerLocation).GetSafeNormal();
			float Dot = FVector::DotProduct(PlayerToCandidate, PlayerForward);
			if (Dot > BestDot)
			{
				BestDot = Dot;
				BestTarget = Candidate;
			}
		}
	}

	return BestTarget;
}

void UZCLockSpringArmComponent::ToggleLock()
{
	if (IsLocked())
	{
		BreakLock();
		return;
	}

	if (UZCLockTargetComponent* Target = FindBestTarget())
	{
		LockTarget(Target);
	}
	else
	{
		ShouldAlignCameraToForward = true;
		ForwardAlignRot = UKismetMathLibrary::MakeRotFromX(GetForwardVector());
	}
}

void UZCLockSpringArmComponent::SwitchTarget(ETargetSwitchDirection SwitchDirection)
{
	if (IsLocked() == false) return;

	TArray<UZCLockTargetComponent*> TargetCandidates = GetTargetComponents(GetComponentLocation(), MaxTargetLockDistance);
	if (TargetCandidates.Num() < 2) return;

	FRotator CameraRotation = GetPlayerCameraManager()->GetCameraRotation();
	FVector NewDirection;
	switch (SwitchDirection)
	{
	case ETargetSwitchDirection::Right:
		NewDirection = CameraRotation.RotateVector(FVector::RightVector);
		break;
	case ETargetSwitchDirection::Up:
		NewDirection = CameraRotation.RotateVector(FVector::UpVector);
		break;
	case ETargetSwitchDirection::Left:
		NewDirection = CameraRotation.RotateVector(FVector::LeftVector);
		break;
	case ETargetSwitchDirection::Down:
		NewDirection = CameraRotation.RotateVector(FVector::DownVector);
		break;
	}

	float BestScore = 0.0f;
	UZCLockTargetComponent* BestCandidate = nullptr;
	for (UZCLockTargetComponent* Candidate : TargetCandidates)
	{
		if (Candidate == CurrentTarget) continue;

		FVector CurrentToCandidate = Candidate->GetComponentLocation() - CurrentTarget->GetComponentLocation();
		float Dot = FVector::DotProduct(CurrentToCandidate.GetSafeNormal(), NewDirection);
		float Dist = CurrentToCandidate.SizeSquared();
		float Score = Dot / Dist;
		if (Score > BestScore)
		{
			BestScore = Score;
			BestCandidate = Candidate;
		}
	}

	if (BestCandidate)
	{
		LockTarget(BestCandidate);
	}
}

void UZCLockSpringArmComponent::LockTarget(UZCLockTargetComponent* TargetComponent)
{
	if (!TargetComponent) return;

	if (CurrentTarget && CurrentTarget == TargetComponent) return;

	if (CurrentTarget) BreakLock();

	CurrentTarget = TargetComponent;
	CurrentTarget->Show();

	Restore_bOrientRotationToMovement = OwningPlayer->GetCharacterMovement()->bOrientRotationToMovement;
	Restore_bUseControllerDesiredRotation = OwningPlayer->GetCharacterMovement()->bUseControllerDesiredRotation;
}

void UZCLockSpringArmComponent::BreakLock()
{
	if (CurrentTarget)
	{
		CurrentTarget->Hide();
	}

	CurrentTarget = nullptr;

	OwningPlayer->GetCharacterMovement()->bOrientRotationToMovement = Restore_bOrientRotationToMovement;
	OwningPlayer->GetCharacterMovement()->bUseControllerDesiredRotation = Restore_bUseControllerDesiredRotation;
}

TArray<UZCLockTargetComponent*> UZCLockSpringArmComponent::GetTargetComponents(FVector SourceLocation, float MaxDistance, bool Unfiltered)
{
	TArray<UPrimitiveComponent*> TargetPrims;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { EObjectTypeQuery::ObjectTypeQuery2 };
	UKismetSystemLibrary::SphereOverlapComponents(GetOwner(), SourceLocation, MaxDistance, ObjectTypes, UZCLockTargetComponent::StaticClass(), TArray<AActor*>{GetOwner()}, TargetPrims);

	TArray<UZCLockTargetComponent*> TargetComponents;
	TargetComponents.Reserve(TargetPrims.Num());

	// 모든 타겟을 가져오는 경우
	if (Unfiltered)
	{
		for (auto* Prim : TargetPrims)
		{
			TargetComponents.Add(Cast<UZCLockTargetComponent>(Prim));
		}
	}
	// 카메라 시야 안에 있는 경우만 
	else
	{
		APlayerCameraManager* CameraManager = GetPlayerCameraManager();
		for (UPrimitiveComponent* Prim : TargetPrims)
		{
			// 카메라 시야 안에 있는지 확인
			FVector CameraToPrim = (Prim->GetComponentLocation() - CameraManager->GetCameraLocation()).GetSafeNormal();
			float CameraDot = FVector::DotProduct(CameraToPrim, CameraManager->GetCameraRotation().RotateVector(FVector::ForwardVector));
			if (CameraDot < 0.0f) continue;

			// 위/ 아래 각도 확인, 13도가 넘어가면 제외
			FVector PlayerToPrim = (Prim->GetComponentLocation() - GetComponentLocation()).GetSafeNormal();
			float VerticalDot = FVector::DotProduct(PlayerToPrim, GetUpVector());
			if (FMath::Abs(VerticalDot) > 0.974f) continue;

			FHitResult HitResult;
			FCollisionQueryParams TraceParms(FName(TEXT("")), false, GetOwner());
			TraceParms.AddIgnoredActor(Prim->GetOwner());

			// 카메라와 타겟 사이에 장애물이 있는지 확인
			if (!GetWorld()->LineTraceSingleByChannel(HitResult, CameraManager->GetCameraLocation(), Prim->GetComponentLocation(), CameraToTarget, TraceParms))
			{
				TargetComponents.Add(Cast<UZCLockTargetComponent>(Prim));
			}

		}
	}

	return TargetComponents;
}

FTransform UZCLockSpringArmComponent::GetTargetTransform() const
{
	return CurrentTarget ? CurrentTarget->GetComponentTransform() : FTransform::Identity;
}

void UZCLockSpringArmComponent::UpdateSocketOffset(float DeltaTime)
{
	// 현재 오프셋과 목표 오프셋이 거의 같으면 연산하지 않음 (성능 최적화)
	if (SocketOffset.Equals(TargetSocketOffset, 0.01f)) return;

	// VInterpTo를 사용하여 부드럽게 보간 (Ease-out 효과)
	// SocketOffsetInterpSpeed가 높을수록 빠르게 도달
	SocketOffset = FMath::VInterpTo(SocketOffset, TargetSocketOffset, DeltaTime, SocketOffsetInterpSpeed);
}
