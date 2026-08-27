// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/State/Monster/ZCMonsterStateComponent.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Game/Subsystem/ZCChemistryGISubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCMonsterStateComponent)

UZCMonsterStateComponent::UZCMonsterStateComponent()
{

}

void UZCMonsterStateComponent::SetArmor()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	const FZCMonsterValue* const *Value = UZCChemistryGISubsystem::Get(this).GetMonsterMap().Find(CharacterTag);

	if (!Value) return;

	const FZCMonsterValue* MonsterValue = *Value;

	Armor.Threshold = &MonsterValue->Threshold;
	Armor.WeakTag = &MonsterValue->WeakTag;
	Armor.StrongTag = &MonsterValue->StrongTag;
	Armor.ImmuntTag = &MonsterValue->ImmuntTag;

	// TODO : Armor의 ArmorState을 정하는 로직 만들어야함
}
