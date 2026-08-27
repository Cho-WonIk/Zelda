// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimInstance/Player/ZCPlayerAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/Player/ZCPlayerCharacter.h"
#include "Component/Movement/ZCCharacterMovementComponent.h"

UZCPlayerAnimInstance::UZCPlayerAnimInstance()
{
}

FVector2D UZCPlayerAnimInstance::VelocityTo2DClimbBlendSpace() const
{
	if (!Player) return FVector2D::ZeroVector;

	// 월드 속도를 캐릭터 로컬 기준으로 변환
	const FVector LocalVelocity = Player->GetActorRotation().UnrotateVector(Velocity);

	// X: 좌우 (Y축), Y: 상하 (X축) 로 변환
	return FVector2D(LocalVelocity.Y, LocalVelocity.Z);
}

FVector2D UZCPlayerAnimInstance::VelocityTo2DGlideBlendSpace() const
{
	if (!Player) return FVector2D::ZeroVector;

	// 월드 속도를 캐릭터 로컬 기준으로 변환
	const FVector LocalVelocity = Player->GetActorRotation().UnrotateVector(Velocity);

	// X: 앞뒤 (X축), Y: 좌우 (Y축)
	return FVector2D(LocalVelocity.X, LocalVelocity.Y);
}

void UZCPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Player = Cast<AZCPlayerCharacter>(TryGetPawnOwner());
	if (Player)
	{
		ZCMovementComponent = Cast<UZCCharacterMovementComponent>(Character->GetCharacterMovement());

		bIsClimbing = ZCMovementComponent->IsClimbing();
		bIsClimbDashing = ZCMovementComponent->IsClimbDashing();

		bIsGliding = ZCMovementComponent->IsGliding();

		bIsCrouching = ZCMovementComponent->IsCrouching();

		bIsRidingGear = ZCMovementComponent->IsRidingGear();
	}
}

void UZCPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (ZCMovementComponent)
	{
		bIsCrouching = ZCMovementComponent->IsCrouching();
	}

	UpdateClimbAnimation();
	UpdateGlideAnimation();
	UpdateRideGearAnimation();
}

void UZCPlayerAnimInstance::UpdateClimbAnimation()
{
	if (ZCMovementComponent)
	{
		bIsClimbing = ZCMovementComponent->IsClimbing();
		bIsClimbDashing = ZCMovementComponent->IsClimbDashing();

		ClimbVector = VelocityTo2DClimbBlendSpace();
		ClimbDashVector = VelocityTo2DClimbBlendSpace();
	}
}

void UZCPlayerAnimInstance::UpdateGlideAnimation()
{
	if (ZCMovementComponent)
	{
		bIsGliding = ZCMovementComponent->IsGliding();
		bHandIK = bIsGliding;

		GlidingOffset = VelocityTo2DGlideBlendSpace();

		// 글라이딩 시 손 위치 조정
		LeftHandIK = Player->GetGlideLeftHandIK();
		RightHandIK = Player->GetGlideRightHandIK();
	}
}

void UZCPlayerAnimInstance::UpdateRideGearAnimation()
{
	if (ZCMovementComponent)
	{
		bIsRidingGear = ZCMovementComponent->IsRidingGear();
	}
}

