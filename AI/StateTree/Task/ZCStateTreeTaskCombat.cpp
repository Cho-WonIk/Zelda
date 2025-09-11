// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Task/ZCStateTreeTaskCombat.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeExecutionTypes.h"
#include "Kismet/GameplayStatics.h"

#include "Character/Monster/ZCMonsterCharacter.h"
#include "AI/ZCAIControllerBase.h"
#include "Actor/Item/Weapon/ZCWeaponActor.h"
#include "Actor/Item/Shield/ZCShieldActor.h"

#include "Interface/ZCAIInterface.h"


////////////////////////////////////////////////////////////////////////////////////////
// ZC 무기 검색 함수
EStateTreeRunStatus FZCStateTreeFindWeaponTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AIController || !InstanceData.MonsterCharacter) return EStateTreeRunStatus::Failed;

	const FVector AIPosition = InstanceData.MonsterCharacter->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));

	TArray<AActor*> FoundWeapons;
	UKismetSystemLibrary::SphereOverlapActors(InstanceData.AIController, AIPosition, InstanceData.WeaponSearchRadius, ObjectTypes, AZCWeaponActor::StaticClass(), {}, FoundWeapons);
	if (FoundWeapons.Num() == 0) return EStateTreeRunStatus::Failed;

	TArray<AActor*> ValidWeapons;
	for (AActor* Actor : FoundWeapons)
	{
		AZCWeaponActor* Weapon = Cast<AZCWeaponActor>(Actor);
		if (Weapon && Weapon->GetOwner() == nullptr)
		{
			ValidWeapons.Add(Weapon);
		}
	}

	if (ValidWeapons.Num() == 0)
	{
		return EStateTreeRunStatus::Failed;
	}

	float NearestDistance = 0.0f;
	AActor* NearestWeapon = UGameplayStatics::FindNearestActor(AIPosition, ValidWeapons, NearestDistance);
	AZCWeaponActor* WeaponActor = Cast<AZCWeaponActor>(NearestWeapon);
	if (!WeaponActor) return EStateTreeRunStatus::Failed;

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::StateTree;
	if (bDrawDebugAll || bDrawTask)
	{
		DrawDebugLine(InstanceData.AIController->GetWorld(), AIPosition, WeaponActor->GetActorLocation(), FColor::Green, false, 1.0f, 0, 2.0f);
	}
#endif // !UE_BUILD_SHIPPING

	AZCWeaponActor** WeaponActorPtr = InstanceData.WeaponActorProperty.GetMutablePtr<AZCWeaponActor*>(Context);
	*WeaponActorPtr = WeaponActor;

	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FZCStateTreeFindWeaponTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 무기 검색 함수"));
}
#endif // WITH_EDITOR


//////////////////////////////////////////////////////////////////////////////////////////
// ZC 방패 검색 함수
EStateTreeRunStatus FZCStateTreeFindShieldTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AIController || !InstanceData.MonsterCharacter) return EStateTreeRunStatus::Failed;

	const FVector AIPosition = InstanceData.MonsterCharacter->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));

	TArray<AActor*> FoundShields;
	UKismetSystemLibrary::SphereOverlapActors(InstanceData.AIController, AIPosition, InstanceData.ShieldSearchRadius, ObjectTypes, AZCShieldActor::StaticClass(), {}, FoundShields);
	if (FoundShields.Num() == 0) return EStateTreeRunStatus::Failed;

	float NearestDistance = 0.0f;
	AActor* NearestShield = UGameplayStatics::FindNearestActor(AIPosition, FoundShields, NearestDistance);
	AZCShieldActor* ShieldActor = Cast<AZCShieldActor>(NearestShield);
	if (!ShieldActor) return EStateTreeRunStatus::Failed;

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::StateTree;
	if (bDrawDebugAll || bDrawTask)
	{
		DrawDebugLine(InstanceData.AIController->GetWorld(), AIPosition, ShieldActor->GetActorLocation(), FColor::Green, false, 1.0f, 0, 2.0f);
	}
#endif // !UE_BUILD_SHIPPING

	AZCShieldActor** ShieldActorPtr = InstanceData.ShieldActorProperty.GetMutablePtr<AZCShieldActor*>(Context);
	*ShieldActorPtr = ShieldActor;

	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FZCStateTreeFindShieldTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 방패 검색 함수"));
}
#endif // WITH_EDITOR


////////////////////////////////////////////////////////////////////////////////////////
// ZC 무기 장착 함수
EStateTreeRunStatus FZCStateTreeEquipWeaponTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.AIController || !InstanceData.MonsterCharacter || !InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;

	if (InstanceData.WeaponActor)
	{
		InstanceData.MonsterCharacter->EquipWeaponByAI(InstanceData.WeaponActor);
	}

	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FZCStateTreeEquipWeaponTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 무기 장착 함수"));
}
#endif // WITH_EDITOR


////////////////////////////////////////////////////////////////////////////////////////
// ZC 방패 장착 함수
EStateTreeRunStatus FZCStateTreeEquipShieldTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.AIController || !InstanceData.MonsterCharacter || !InstanceData.ShieldActor) return EStateTreeRunStatus::Failed;
	if (InstanceData.ShieldActor)
	{
	}
	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FZCStateTreeEquipShieldTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 방패 장착 함수"));
}
#endif // WITH_EDITOR


////////////////////////////////////////////////////////////////////////////////////////
// ZC 공격 함수
EStateTreeRunStatus FZCStateTreeAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AIController || !InstanceData.MonsterCharacter || !InstanceData.TargetActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	IZCAIInterface::Execute_AttackByAI(InstanceData.MonsterCharacter, InstanceData.TargetActor->GetActorLocation());

	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FZCStateTreeAttackTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 공격 함수"));
}
#endif // WITH_EDITOR


////////////////////////////////////////////////////////////////////////////////////////
// ZC 방어 함수
EStateTreeRunStatus FZCStateTreeDefenseTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AIController || !InstanceData.MonsterCharacter || !InstanceData.TargetActor) return EStateTreeRunStatus::Failed;

	IZCAIInterface::Execute_DefenseByAI(InstanceData.MonsterCharacter, InstanceData.TargetActor->GetActorLocation());

	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FZCStateTreeDefenseTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 방어 함수"));
}
#endif // WITH_EDITOR