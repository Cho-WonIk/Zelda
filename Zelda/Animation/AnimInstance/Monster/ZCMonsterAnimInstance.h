// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance/ZCCharacterAnimInstance.h"
#include "ZCMonsterAnimInstance.generated.h"

class AZCMonsterCharacter;

UCLASS()
class ZELDA_API UZCMonsterAnimInstance : public UZCCharacterAnimInstance
{
	GENERATED_BODY()

public:
	UZCMonsterAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class AZCMonsterCharacter> Monster;
};
