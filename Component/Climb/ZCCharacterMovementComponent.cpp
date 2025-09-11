// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Climb/ZCCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "ZCClimbTrace.h"

#include "Development/ZCDebug.h"
#if !UE_BUILD_SHIPPING
namespace Zelda::Debug::Climb
{
	static bool bDrawDebugAll = false;
	static bool bDrawPlayer = false;
	static bool bDrawHitResult = false;
	static bool bDrawLedge = false;

	static FAutoConsoleVariableRef CVar_DebugAll(Zelda::Debug::Climb::all, bDrawDebugAll, TEXT("Climb 디버깅 전체 On/Off"), ECVF_Default);
	static FAutoConsoleVariableRef CVar_DebugPlayer(Zelda::Debug::Climb::player, bDrawPlayer, TEXT("Climb - 플레이어 캡슐 시각화"), ECVF_Default);
	static FAutoConsoleVariableRef CVar_DebugHit(Zelda::Debug::Climb::hitresult, bDrawHitResult, TEXT("Climb - 벽 충돌 지점 표시"), ECVF_Default);
	static FAutoConsoleVariableRef CVar_DebugLedge(Zelda::Debug::Climb::ledge, bDrawLedge, TEXT("Climb - ledge 트레이스 표시"), ECVF_Default);
}
#endif

UZCCharacterMovementComponent::UZCCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UZCCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	ClimbQueryParams.AddIgnoredActor(GetOwner());
}

void UZCCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckCliffEdge();

	// 표면등반 표면 탐색
	SweepAndStoreWallHits();
}

void UZCCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	if (bWantsToClimb)
	{
		SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Climbing);
	}
	else if (bWantsToGliding)
	{
		SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Gliding);
	}

}

void UZCCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	// 글라이딩 상태 입장시 설정
	if (IsGliding())
	{
		OnMovementChangeDelegate.Broadcast(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Gliding);

		Restore_GravityScale = GravityScale;
		GravityScale = GlidingGravityScale;

		Restore_bOrientRotationToMovement = bOrientRotationToMovement;
		Restore_RotationRate = RotationRate;
		bOrientRotationToMovement = true;
		RotationRate = GlidingRotationRate;

		// Z방향 속도 제거하고 수평속도를 MaxGlidingSpeed로 제한
		Velocity = ProjectToGravityFloor(Velocity).GetClampedToMaxSize(MaxGlidingSpeed);

	}

	// 글라이딩 상태에서 원상태 복귀시 설정
	const bool bWasGliding = PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Gliding;
	if (bWasGliding)
	{
		OnMovementChangeDelegate.Broadcast(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_MAX);

		bOrientRotationToMovement = Restore_bOrientRotationToMovement;
		RotationRate = Restore_RotationRate;
		GravityScale = Restore_GravityScale;
		SetRotationToStand();
		StopMovementImmediately();
	}
	
	// 표면등반 상태 입장시 설정
	if (IsClimbing())
	{
		OnMovementChangeDelegate.Broadcast(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Climbing);

		Restore_bOrientRotationToMovement = bOrientRotationToMovement;
		bOrientRotationToMovement = false;

		UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
		Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight() - ClimbingCollisionShrinkAmount);
	}

	// 표면등반 상태에서 원상태 복귀시 설정
	const bool bWasClimbing = PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Climbing;
	if (bWasClimbing)
	{
		OnMovementChangeDelegate.Broadcast(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_MAX);

		bOrientRotationToMovement = Restore_bOrientRotationToMovement;

		SetRotationToStand();

		UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
		Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight() + ClimbingCollisionShrinkAmount);

		StopMovementImmediately();

		LastClimbingEndTime = GetWorld()->GetTimeSeconds();
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if(MovementMode != MOVE_Custom)
	{
		OnMovementChangeDelegate.Broadcast(MovementMode, ECustomMovementMode::CMOVE_MAX);
	}

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::Climb;

	if (bDrawDebugAll && GEngine && CharacterOwner)
	{
		const UEnum* MovementEnum = StaticEnum<EMovementMode>();
		const UEnum* CustomEnum = StaticEnum<ECustomMovementMode>();

		FString PrevModeStr;
		PrevModeStr = PreviousMovementMode == MOVE_Custom
			? FString::Printf(TEXT("Custom::%s"), *CustomEnum->GetValueAsString(static_cast<ECustomMovementMode>(PreviousCustomMode)))
			: MovementEnum->GetValueAsString(PreviousMovementMode);

		FString CurrModeStr;
		CurrModeStr = PreviousMovementMode == MOVE_Custom
			? FString::Printf(TEXT("Custom::%s"), *CustomEnum->GetValueAsString(static_cast<ECustomMovementMode>(CustomMovementMode)))
			: MovementEnum->GetValueAsString(MovementMode);

		GEngine->AddOnScreenDebugMessage(9991, 1.5f, FColor::Yellow, FString::Printf(TEXT("Movement Mode Changed\n%s → %s"), *PrevModeStr, *CurrModeStr));
	}
#endif
}

float UZCCharacterMovementComponent::GetMaxSpeed() const
{
	if (IsClimbing()) return MaxClimbingSpeed;
	if (IsGliding()) return MaxGlidingSpeed;
	return Super::GetMaxSpeed();
}

float UZCCharacterMovementComponent::GetMaxAcceleration() const
{
	if (IsClimbing()) return MaxClimbingAcceleration;
	if (IsGliding()) return MaxGlidingAcceleration;
	return Super::GetMaxAcceleration();
}

void UZCCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (CustomMovementMode == ECustomMovementMode::CMOVE_Climbing)
	{
		PhysClimbing(DeltaTime, Iterations);
	}

	if (CustomMovementMode == ECustomMovementMode::CMOVE_Gliding)
	{
		PhysGliding(DeltaTime, Iterations);
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

bool UZCCharacterMovementComponent::CheckFloor(FHitResult& FloorHit) const
{
	const FVector Start = UpdatedComponent->GetComponentLocation() + (UpdatedComponent->GetUpVector() * -20);
	const FVector End = Start + FVector::DownVector * FloorCheckDistance;

	// 구체 반지름 설정
	const float SphereRadius = 30.0f;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(SphereRadius);

	return GetWorld()->SweepSingleByChannel(FloorHit, Start, End, FQuat::Identity, ECC_Visibility, Sphere, ClimbQueryParams);
}

/* ======================================= Walk상태 로직들 ========================================= */
void UZCCharacterMovementComponent::StartRunning()
{
	bIsRunning = true;

	Restore_MaxWalkSpeed = MaxWalkSpeed;
	MaxWalkSpeed = RunningSpeed;
}

void UZCCharacterMovementComponent::StopRunning()
{
	bIsRunning = false;
	MaxWalkSpeed = Restore_MaxWalkSpeed;
}

void UZCCharacterMovementComponent::SetAttentionState(bool bIsAttention)
{
	if (bIsAttention)
	{
		Restore_MaxWalkSpeed = MaxWalkSpeed;
		MaxWalkSpeed = AttentionSpeed;
	}
	else
	{
		MaxWalkSpeed = Restore_MaxWalkSpeed;
	}
}

/* ======================================= 글라이딩 로직들 ========================================= */
bool UZCCharacterMovementComponent::CanStartGliding()
{
	// 표면등반 중이거나 수영중 이거나 글라이딩 중이면 불가능
	if (IsClimbing() || IsSwimming() || IsGliding())
	{
		return false;
	}
	return !IsMovingOnGround() && IsFalling();
}

void UZCCharacterMovementComponent::TryGliding()
{
	if (CanStartGliding())
	{
		bWantsToGliding = true;
	}
}

void UZCCharacterMovementComponent::CancelGliding()
{
	bWantsToGliding = false;
}

void UZCCharacterMovementComponent::PhysGliding(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME) return;

	if (ShouldStopGliding() || GlidingDownToFloor() || IsSwimming())
	{
		StopGliding(DeltaTime, Iterations);
		return;
	}

	const FVector FallAcceleration = ProjectToGravityFloor(GetFallingLateralAcceleration(DeltaTime));
	float remainingTime = DeltaTime;

	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
		bJustTeleported = false;

		RestorePreAdditiveRootMotionVelocity();

		// 글라이딩 속도 계산 유지
		Velocity = ComputeGlidingVelocity(timeTick, FallAcceleration);

		FVector Adjusted = Velocity * timeTick;

		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Adjusted, PawnRotation, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			// 슬라이딩 처리
			SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);

			// 슬라이드 후에도 위치가 변경되지 않았다면, 강제로 떨어지게 처리
			if ((UpdatedComponent->GetComponentLocation() - OldLocation).SizeSquared() <= KINDA_SMALL_NUMBER)
			{
				Velocity = FVector::ZeroVector;
				break;
			}
		}
	}
}

bool UZCCharacterMovementComponent::ShouldStopGliding() const
{
	return IsMovingOnGround() || !bWantsToGliding;
}

bool UZCCharacterMovementComponent::GlidingDownToFloor() const
{
	FHitResult FloorHit;

	if (!CheckFloor(FloorHit))
	{
		return false;
	}

	// 걷기 가능한 바닥인가?
	const bool bOnWalkableFloor = FloorHit.Normal.Z > GetWalkableFloorZ();

	// 캐릭터가 그 바닥 방향으로 이동 중인가?
	const float DownSpeed = FVector::DotProduct(Velocity, -FloorHit.Normal);
	const bool bIsMovingDownward = DownSpeed >= 10.0f;

	return bIsMovingDownward && bOnWalkableFloor;
}

void UZCCharacterMovementComponent::StopGliding(float DeltaTime, int32 Iterations)
{
	bWantsToGliding = false;

	bWantsToClimb ? SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Climbing) : SetMovementMode(EMovementMode::MOVE_Falling);

	//SetMovementMode(EMovementMode::MOVE_Falling);
	StartNewPhysics(DeltaTime, Iterations);
}

FVector UZCCharacterMovementComponent::ComputeGlidingVelocity(float DeltaTime, const FVector& FallAcceleration)
{
	FVector ResultVelocity = Velocity;

	// Z축 방향 속도 제거
	FVector HorizontalVelocity = ProjectToGravityFloor(ResultVelocity + FallAcceleration * DeltaTime).GetClampedToMaxSize(MaxGlidingSpeed);

	// Z축 중력 적용 및 제한
	const FVector GravityDir = -GetGravityDirection();
	const float VerticalSpeed = FVector::DotProduct(ResultVelocity, GravityDir);
	const float NewVerticalSpeed = FMath::Clamp(VerticalSpeed + (GetGravityZ() * GlidingGravityScale * DeltaTime), -MaxGlidingFallingSpeed, MaxGlidingFallingSpeed);
	const FVector VerticalVelocity = GravityDir * NewVerticalSpeed;

	return HorizontalVelocity + VerticalVelocity;
}

/* ======================================= 표면등반 로직들 ========================================= */
void UZCCharacterMovementComponent::TryClimbing()
{
	if (CanStartClimbing())
	{
		bWantsToClimb = true;
	}
}

void UZCCharacterMovementComponent::TryClimbDashing()
{
	// 대쉬 중에 중복 호출 방지
	if (ClimbDashCurve && bIsClimbDashing == false)
	{
		bIsClimbDashing = true;
		CurrentClimbDashTime = 0.0f;

		StoreClimbDashDirection();
	}
}

void UZCCharacterMovementComponent::CancelClimbing()
{
	bWantsToClimb = false;
}

bool UZCCharacterMovementComponent::CanStartClimbing()
{
	if (CharacterOwner->bIsCrouched) return false;

	if (LastClimbingEndTime > 0.0f && ((GetWorld()->GetTimeSeconds() - LastClimbingEndTime) < ClimbingCooldownDuration)) return false;

	if (ClimbDownToFloor()) return false;

	for (FHitResult& Hit : CurrentWallHits)
	{
		const FVector HorizontalNormal = Hit.Normal.GetSafeNormal2D();

		const float HorizontalDot = FVector::DotProduct(UpdatedComponent->GetForwardVector(), -HorizontalNormal);
		const float VerticalDot = FVector::DotProduct(Hit.Normal, HorizontalNormal);

		const float HorizontalDegrees = FMath::RadiansToDegrees(FMath::Acos(HorizontalDot));

		const bool bIsCeiling = FMath::IsNearlyZero(VerticalDot);

		const bool bIsHittingLedge = HasReachedEdge() && CanMoveToLedgeClimbLocation();

		if ((HorizontalDegrees <= MinHorizontalDegreesToStartClimbing && !bIsCeiling && IsFacingSurface(VerticalDot)) || !bIsHittingLedge)
		{
			return true;
		}
	}

	return false;
}

void UZCCharacterMovementComponent::TryClimbDownCliff()
{
	if (AnimInstance && AnimInstance->Montage_IsPlaying(CliffClimbMontage)) return;

	if (CheckCliffEdge())
	{

		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
			{
				if (bInterrupted) return;
				// 낭떠러지에서 클라이밍 대쉬
				TryClimbing();
			});
		AnimInstance->Montage_Play(CliffClimbMontage, 1.0f);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, CliffClimbMontage);

	}
}

void UZCCharacterMovementComponent::SweepAndStoreWallHits()
{
	const FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CollisionCapsuleRadius, CollisionCapsuleHalfHeight);

	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 20;

	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	TArray<FHitResult> Hits;
	const bool HitWall = GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ClimbTrace, CollisionShape, ClimbQueryParams);

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::Climb;

	if (bDrawDebugAll || bDrawPlayer)
	{
		DrawDebugCapsule(GetWorld(), UpdatedComponent->GetComponentLocation(), CollisionCapsuleHalfHeight, CollisionCapsuleRadius, FQuat::Identity, FColor::White, false, 0.0f);
	}

	if ((bDrawDebugAll || bDrawHitResult) && HitWall)
	{
		for (const FHitResult& Hit : Hits)
		{
			DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 12.0f, FColor::Green, false, 1.0f);
		}
	}
#endif

	if (HitWall)
	{
		CurrentWallHits = Hits;
	}
	else
	{
		CurrentWallHits.Reset();
	}
}

bool UZCCharacterMovementComponent::CheckCliffEdge()
{
	if (!IsMovingOnGround()) return false;

	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Down = FVector::DownVector;

	// 1. 현재 위치 아래로 Climbable 채널 트레이스 (현재 바닥이 클라이밍 가능한가?)
	FHitResult ClimbableHit;
	const FVector BaseTraceStart = CurrentLocation;
	const FVector BaseTraceEnd = BaseTraceStart + Down * 100.0f;

	const bool bOnClimbableSurface = GetWorld()->LineTraceSingleByChannel(ClimbableHit, BaseTraceStart, BaseTraceEnd, ClimbTrace, ClimbQueryParams);

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::Climb;

	if (bDrawDebugAll || bDrawLedge)
	{
		DrawDebugLine(GetWorld(), BaseTraceStart, BaseTraceEnd, bOnClimbableSurface ? FColor::Green : FColor::Red, false, 1.0f, 0, 0.0f);
		if (bOnClimbableSurface)
		{
			DrawDebugPoint(GetWorld(), ClimbableHit.ImpactPoint, 10.0f, FColor::Green, false, 1.0f);
		}
	}
#endif

	// 2. 약간 전방에서 ECC_Visibility 채널로 아래 방향 트레이스 (전방 바닥 없음 확인)
	FHitResult ForwardHit;
	FVector ForwardOffsetStart = CurrentLocation + Forward * 30.0f;
	FVector ForwardOffsetEnd = ForwardOffsetStart + Down * 200.0f;

	const bool bHasForwardGround = GetWorld()->LineTraceSingleByChannel(ForwardHit, ForwardOffsetStart, ForwardOffsetEnd, ECC_Visibility, ClimbQueryParams);

	// 3. 다시 캐릭터 쪽으로 트레이스(표면 정보 얻기)
	ForwardOffsetStart = ForwardOffsetStart + Down * 200.0f;
	ForwardOffsetEnd = ForwardOffsetStart + (-Forward) * 30.0f;

	const bool bHasClimbableSurface = GetWorld()->LineTraceSingleByChannel(ForwardHit, ForwardOffsetStart, ForwardOffsetEnd, ClimbTrace, ClimbQueryParams);

#if !UE_BUILD_SHIPPING
	if (bDrawDebugAll || bDrawLedge)
	{
		DrawDebugLine(GetWorld(), ForwardOffsetStart, ForwardOffsetEnd, bHasClimbableSurface ? FColor::Red : FColor::Green, false, 1.0f, 0, 0.0f);
		if (bHasClimbableSurface)
		{
			DrawDebugPoint(GetWorld(), ForwardHit.ImpactPoint, 10.0f, FColor::Red, false, 1.0f);
		}
	}
#endif
	bool bCanClimbDown = bOnClimbableSurface && !bHasForwardGround && bHasClimbableSurface;

	bCanClimbDown ? ClimbClimbDownSurface = ForwardHit.ImpactPoint : ClimbClimbDownSurface = FVector::ZeroVector;

	OnCanClimbDownCliffDelegate.ExecuteIfBound(bCanClimbDown);

	return bCanClimbDown;
}

void UZCCharacterMovementComponent::ComputeSurfaceInfo()
{
	CurrentClimbingNormal = FVector::ZeroVector;
	CurrentClimbingPosition = FVector::ZeroVector;

	if (CurrentWallHits.IsEmpty()) return;

	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(6);

	for (const FHitResult& WallHit : CurrentWallHits)
	{
		const FVector End = Start + (WallHit.ImpactPoint - Start).GetSafeNormal() * 120;

		FHitResult AssistHit;
		GetWorld()->SweepSingleByChannel(AssistHit, Start, End, FQuat::Identity, ClimbTrace, CollisionSphere, ClimbQueryParams);

		CurrentClimbingPosition += AssistHit.Location;
		CurrentClimbingNormal += AssistHit.Normal;
	}

	CurrentClimbingPosition /= CurrentWallHits.Num();
	CurrentClimbingNormal = CurrentClimbingNormal.GetSafeNormal();
}

bool UZCCharacterMovementComponent::EyeHeightTrace(const float TraceDistance) const
{
	FHitResult UpperEdgeHit;

	const float BaseEyeHeight = GetCharacterOwner()->BaseEyeHeight;
	const float EyeHeightOffset = IsClimbing() ? BaseEyeHeight + ClimbingCollisionShrinkAmount : BaseEyeHeight;

	const FVector Start = UpdatedComponent->GetComponentLocation() + UpdatedComponent->GetUpVector() * EyeHeightOffset;
	const FVector End = Start + (UpdatedComponent->GetForwardVector() * TraceDistance);

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::Climb;

	if (bDrawDebugAll || bDrawLedge)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.0f);
	}
#endif

	return GetWorld()->LineTraceSingleByChannel(UpperEdgeHit, Start, End, ClimbTrace, ClimbQueryParams);
}

bool UZCCharacterMovementComponent::IsFacingSurface(float Steepness) const
{
	constexpr float BaseLength = 60;
	const float SteepnessMultiplier = 1 + (1 - Steepness) * 5;

	return EyeHeightTrace(BaseLength * SteepnessMultiplier);
}

void UZCCharacterMovementComponent::SnapToClimbingSurface(float DeltaTime) const
{
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Location = UpdatedComponent->GetComponentLocation();
	const FQuat Rotation = UpdatedComponent->GetComponentQuat();

	const FVector ForwardDifference = (CurrentClimbingPosition - Location).ProjectOnTo(Forward);

	const FVector Offset = -CurrentClimbingNormal * (ForwardDifference.Length() - DistanceFromSurface);

	constexpr bool bSweep = true;

	const float SnapSpeed = ClimbingSnapSpeed * ((Velocity.Length() / MaxClimbingSpeed) + 1);
	UpdatedComponent->MoveComponent(Offset * SnapSpeed * DeltaTime, Rotation, bSweep);
}

void UZCCharacterMovementComponent::PhysClimbing(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME) return;

	ComputeSurfaceInfo();	// 현재 표면 검사

	// 종료 조건 검사
	if (ShouldStopClimbing() || ClimbDownToFloor() || IsSwimming() || IsGliding())
	{
		StopClimbing(DeltaTime, Iterations);
		return;
	}

	UpdateClimbDashState(DeltaTime);	// 대시 상태 업데이트

	ComputeClimbingVelocity(DeltaTime);	// 속도 계산

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();

	MoveAlongClimbingSurface(DeltaTime);	// 표면 정보를 바탕으로 이동

	TryClimbUpLedge();	// ledge인지 검사 후 ledge이면 클라이밍 종료

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
	}

	SnapToClimbingSurface(DeltaTime);	// 표면의 법선 벡터에 맞춰 정렬
}

void UZCCharacterMovementComponent::ComputeClimbingVelocity(float DeltaTime)
{
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		if (bIsClimbDashing)
		{
			AlignClimbDashDirection();

			const float CurrentCurveSpeed = ClimbDashCurve->GetFloatValue(CurrentClimbDashTime);
			Velocity = ClimbDashDirection * CurrentCurveSpeed;
		}
		else
		{
			constexpr float Friction = 0.0f;
			constexpr bool bFluid = false;
			CalcVelocity(DeltaTime, Friction, bFluid, BrakingDecelerationClimbing);
		}
	}

	ApplyRootMotionToVelocity(DeltaTime);
}

void UZCCharacterMovementComponent::MoveAlongClimbingSurface(float DeltaTime)
{
	const FVector Adjusted = Velocity * DeltaTime;

	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Adjusted, GetClimbingRotation(DeltaTime), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}
}

bool UZCCharacterMovementComponent::ClimbDownToFloor() const
{
	FHitResult FloorHit;
	if (!CheckFloor(FloorHit))
	{
		return false;
	}

	// 걷기 가능한 바닥인가
	const bool bOnWalkableFloor = FloorHit.Normal.Z > GetWalkableFloorZ();

	// 캐릭터가 그 바닥으로 이동 중인지 확인
	const float DownSpeed = FVector::DotProduct(Velocity, -FloorHit.Normal);
	const bool bIsMovingTowardsFloor = DownSpeed >= MaxClimbingSpeed / 3 && bOnWalkableFloor;

	const bool bIsClimbingFloor = CurrentClimbingNormal.Z > GetWalkableFloorZ();
	return bIsMovingTowardsFloor || (bIsClimbingFloor && bOnWalkableFloor);
}

void UZCCharacterMovementComponent::SetRotationToStand() const
{
	const FRotator StandRotation = FRotator(0, UpdatedComponent->GetComponentRotation().Yaw, 0);
	UpdatedComponent->SetRelativeRotation(StandRotation);
}

bool UZCCharacterMovementComponent::TryClimbUpLedge() const
{
	if (AnimInstance && AnimInstance->Montage_IsPlaying(LedgeClimbMontage))
	{
		return false;
	}

	const float UpSpeed = FVector::DotProduct(Velocity, UpdatedComponent->GetUpVector());
	const bool bIsMovingUp = UpSpeed >= MaxClimbingSpeed / 3;

	if (bIsMovingUp && HasReachedEdge() && CanMoveToLedgeClimbLocation())
	{
		SetRotationToStand();

		AnimInstance->Montage_Play(LedgeClimbMontage);

		return true;
	}

	return false;
}

bool UZCCharacterMovementComponent::HasReachedEdge() const
{
	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float TraceDistance = Capsule->GetUnscaledCapsuleRadius() * 2.5f;

	return !EyeHeightTrace(TraceDistance);
}

bool UZCCharacterMovementComponent::CanMoveToLedgeClimbLocation() const
{
	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();

	const FVector VerticalOffset = FVector::UpVector * 160.f;
	const FVector HorizontalOffset = UpdatedComponent->GetForwardVector() * 100.f;

	const FVector CheckLocation = UpdatedComponent->GetComponentLocation() + HorizontalOffset + VerticalOffset;

	if (!IsLocationWalkable(CheckLocation))
	{
		return false;
	}

	FHitResult CapsuleHit;
	const FVector CapsuleStartCheck = CheckLocation - HorizontalOffset;

	const bool bBlocked = GetWorld()->SweepSingleByChannel(CapsuleHit, CapsuleStartCheck, CheckLocation, FQuat::Identity, ClimbTrace, Capsule->GetCollisionShape(), ClimbQueryParams);

	return !bBlocked;
}

bool UZCCharacterMovementComponent::IsLocationWalkable(const FVector& CheckLocation) const
{
	const FVector CheckEnd = CheckLocation + (FVector::DownVector * 250);

	FHitResult LedgeHit;
	const bool bHitLedgeGround = GetWorld()->LineTraceSingleByChannel(LedgeHit, CheckLocation, CheckEnd, ECC_WorldStatic, ClimbQueryParams);

	return bHitLedgeGround && LedgeHit.Normal.Z >= GetWalkableFloorZ();
}

bool UZCCharacterMovementComponent::ShouldStopClimbing() const
{
	if (AnimInstance && AnimInstance->Montage_IsPlaying(LedgeClimbMontage))
	{
		return false;
	}

	// 천장을 만난 경우
	const bool bIsOnCeiling = FVector::Parallel(CurrentClimbingNormal, FVector::UpVector);

	return !bWantsToClimb || CurrentClimbingNormal.IsZero() || bIsOnCeiling;
}

void UZCCharacterMovementComponent::StopClimbing(float DeltaTime, int32 Iterations)
{
	StopClimbDashing();

	bWantsToClimb = false;
	SetMovementMode(EMovementMode::MOVE_Falling);
	StartNewPhysics(DeltaTime, Iterations);
}

FQuat UZCCharacterMovementComponent::GetClimbingRotation(float DeltaTime) const
{
	const FQuat Current = UpdatedComponent->GetComponentQuat();

	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return Current;
	}

	const FQuat Target = FRotationMatrix::MakeFromX(-CurrentClimbingNormal).ToQuat();
	const float RotationSpeed = ClimbingRotationSpeed * FMath::Max(1, Velocity.Length() / MaxClimbingSpeed);

	return FMath::QInterpTo(Current, Target, DeltaTime, RotationSpeed);
}

void UZCCharacterMovementComponent::UpdateClimbDashState(float DeltaTime)
{
	if (!bIsClimbDashing) return;

	CurrentClimbDashTime += DeltaTime;

	// 대시 커브 확인후 종료
	float MinTime, MaxTime;
	ClimbDashCurve->GetTimeRange(MinTime, MaxTime);

	if (CurrentClimbDashTime >= MaxTime)
	{
		StopClimbDashing();
	}
}

void UZCCharacterMovementComponent::AlignClimbDashDirection()
{
	const FVector HorizontalSurfaceNormal = GetClimbSurfaceNormal();

	ClimbDashDirection = FVector::VectorPlaneProject(ClimbDashDirection, HorizontalSurfaceNormal);
}

void UZCCharacterMovementComponent::StoreClimbDashDirection()
{
	ClimbDashDirection = UpdatedComponent->GetUpVector();

	const float AccelerationThreshold = MaxClimbingAcceleration / 10;
	if (Acceleration.Length() > AccelerationThreshold)
	{
		ClimbDashDirection = Acceleration.GetSafeNormal();
	}
}

void UZCCharacterMovementComponent::StopClimbDashing()
{
	bIsClimbDashing = false;
	CurrentClimbDashTime = 0.f;
	ClimbDashDirection = FVector::ZeroVector;
}
