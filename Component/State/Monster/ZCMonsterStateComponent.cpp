// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/State/Monster/ZCMonsterStateComponent.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "World/Subsystem/ZCWorldSubsystem.h"

UZCMonsterStateComponent::UZCMonsterStateComponent()
{

}

void UZCMonsterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	const FCharacterArmorTypeInstanceData* Found = WorldSubsystem->GetCharacterMonsterTypeInstanceData(CharacterTag);
	Armor.ArmorState = Found->ArmorState;
	Armor.ImmutTag = Found->ImmuntTag;
	Armor.StrongTag = Found->StrongTag;
	Armor.Threshold = Found->Threshold;
	Armor.ThresholdDelta = Found->ThresholdDelta;
	Armor.WeakTag = Found->WeakTag;

}
