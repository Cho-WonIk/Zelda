// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimInstance/ZCCharacterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

#include "Component/State/ZCStateComponent.h"

#include "Character/ZCCharacter.h"

UZCCharacterAnimInstance::UZCCharacterAnimInstance()
{
	MovingThreshould = 3.0f;
	JumpingThreshould = 100.0f;
}

void UZCCharacterAnimInstance::NativeInitializeAnimation()
{
	Character = Cast<AZCCharacter>(GetOwningActor());
	if (Character)
	{
		MovementComponent = Character->GetCharacterMovement();

		HitReactionComponent = Character->GetHitReactionComponent();

		Character->OnWeaponTypeChanged.AddLambda([this](EWeaponType NewWeaponType) { WeaponType = NewWeaponType; });
		Character->OnShieldTypeChanged.AddLambda([this](EShieldType NewShieldType) { ShieldType = NewShieldType; });
		Character->OnGuardStateChanged.AddLambda([this](bool bIsGuard) { bIsGuarding = bIsGuard; });

		Character->GetStateComponent()->OnHealthZero.AddLambda([this]() { bIsDeath = true; });

		WeaponType = EWeaponType::None;
		ShieldType = EShieldType::None;
		bIsGuarding = false;

		bIsDeath = false;

		if (MovementComponent)
		{
			Velocity = MovementComponent->Velocity;
			Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
			GroundSpeed = Velocity.Size2D();

			bIsIdle = GroundSpeed < MovingThreshould;
			bIsFalling = MovementComponent->IsFalling();

			bIsJumping = bIsFalling & (Velocity.Z > JumpingThreshould);
		}
	}
}

void UZCCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (Character && MovementComponent)
	{
		Velocity = MovementComponent->Velocity;
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
		GroundSpeed = Velocity.Size2D();

		bIsIdle = GroundSpeed < MovingThreshould;
		bIsFalling = MovementComponent->IsFalling();

		bIsJumping = bIsFalling & (Velocity.Z > JumpingThreshould);
	}
}
