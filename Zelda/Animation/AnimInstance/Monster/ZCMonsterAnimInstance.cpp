// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimInstance/Monster/ZCMonsterAnimInstance.h"
#include "Character/Monster/ZCMonsterCharacter.h"

UZCMonsterAnimInstance::UZCMonsterAnimInstance()
{
}

void UZCMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Monster = Cast<AZCMonsterCharacter>(TryGetPawnOwner());
	if (Monster)
	{
		
	}
}

void UZCMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

}
