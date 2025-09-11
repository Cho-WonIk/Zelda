// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Struct/Enum/ZCItemType.h"
#include "ZCCharacterAnimInstance.generated.h"

class AZCCharacter;
class UZCHitReactionComponent;

UCLASS()
class ZELDA_API UZCCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UZCCharacterAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class AZCCharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UCharacterMovementComponent> MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UZCHitReactionComponent> HitReactionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float GroundSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float Direction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float MovingThreshould;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float JumpingThreshould;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsFalling : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsJumping : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsIdle : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsDeath : 1;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (DisplayName = "무기 종류"))
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (DisplayName = "방패 종류"))
	EShieldType ShieldType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (DisplayName = "가드상태"))
	bool bIsGuarding;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = IK)
	bool bHandIK = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = IK)
	bool bFootIK = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FTransform LeftHandIK;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FTransform RightHandIK;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FTransform LeftFootIK;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FTransform RightFootIK;

};
