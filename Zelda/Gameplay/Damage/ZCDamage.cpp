// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Damage/ZCDamage.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCDamage)

UZCDamageType::UZCDamageType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DamageTypeTag = TAG_DamageType_Blunt;
}