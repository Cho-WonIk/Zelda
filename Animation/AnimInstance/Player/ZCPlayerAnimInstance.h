// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance/ZCCharacterAnimInstance.h"
#include "ZCPlayerAnimInstance.generated.h"

class AZCPlayerCharacter;
class UZCCharacterMovementComponent;

UCLASS()
class ZELDA_API UZCPlayerAnimInstance : public UZCCharacterAnimInstance
{
	GENERATED_BODY()
	
public:
	UZCPlayerAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	void UpdateClimbAnimation();
	void UpdateGlideAnimation();

private:
	FVector2D VelocityTo2DClimbBlendSpace() const;
	FVector2D VelocityTo2DGlideBlendSpace() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class AZCPlayerCharacter> Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UZCCharacterMovementComponent> ZCMovementComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crouch")
	bool bIsCrouching;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gliding")
	bool bIsGliding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gliding")
	FVector2D GlidingOffset;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bIsClimbing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climb")
	FVector2D ClimbVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bIsClimbDashing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climb")
	FVector2D ClimbDashVector;
};
