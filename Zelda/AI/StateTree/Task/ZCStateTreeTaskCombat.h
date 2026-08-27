// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreePropertyRef.h"
#include "Tasks/StateTreeAITask.h"

#include "ZCStateTreeInstanceBase.h"

#include "ZCStateTreeTaskCombat.generated.h"

class AZCWeaponActor;
class AZCShieldActor;

USTRUCT()
struct FZCFindWeaponTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()

	// 무기 검색 범위
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "무기 검색 범위"))
	float WeaponSearchRadius = 500.0f;

	// 무기 검색 결과
	UPROPERTY(VisibleAnywhere, Category = "Out", meta = (RefType = "/Script/Zelda.ZCWeaponActor"))
	FStateTreePropertyRef WeaponActorProperty;
};

USTRUCT(meta = (DisplayName = "ZC 무기 검색", Category = "Zelda|Combat"))
struct FZCStateTreeFindWeaponTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCFindWeaponTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCFindShieldTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()

	// 방패 검색 범위
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "방패 검색 범위"))
	float ShieldSearchRadius = 500.0f;
	// 방패 검색 결과
	UPROPERTY(VisibleAnywhere, Category = "Out", meta = (RefType = "/Script/Zelda.ZCShieldActor"))
	FStateTreePropertyRef ShieldActorProperty;
};

USTRUCT(meta = (DisplayName = "ZC 방패 검색", Category = "Zelda|Combat"))
struct FZCStateTreeFindShieldTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCFindShieldTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCEquipWeaponTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()

	// 장착할 무기 액터
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "장착할 무기 액터"))
	AZCWeaponActor* WeaponActor = nullptr;
};

USTRUCT(meta = (DisplayName = "ZC 무기 장착", Category = "Zelda|Combat"))
struct FZCStateTreeEquipWeaponTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCEquipWeaponTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCEquipShieldTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()

	// 장착할 방패 액터
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "장착할 방패 액터"))
	AZCShieldActor* ShieldActor = nullptr;
};

USTRUCT(meta = (DisplayName = "ZC 방패 장착", Category = "Zelda|Combat"))
struct FZCStateTreeEquipShieldTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()
	using FInstanceDataType = FZCEquipShieldTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCAttackTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()

	// 공격 대상 액터
	UPROPERTY(EditAnywhere, Category = "Parameter")
	AActor* TargetActor = nullptr;
};

USTRUCT(meta = (DisplayName = "ZC 공격 함수", Category = "Zelda|Combat"))
struct FZCStateTreeAttackTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCAttackTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCDefenseTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()
	// 방어 대상 액터
	UPROPERTY(EditAnywhere, Category = "Parameter")
	AActor* TargetActor = nullptr;
};

USTRUCT(meta = (DisplayName = "ZC 방어 함수", Category = "Zelda|Combat"))
struct FZCStateTreeDefenseTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCDefenseTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
