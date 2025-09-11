
#pragma once

#include "AITypes.h"
#include "StateTreeConditionBase.h"
#include "StateTreeAnyEnum.h"
#include "ZCStatTreeConditions.generated.h"

USTRUCT()
struct FZCCompareActorDistanceConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	double Distance = 0.0f;
};
STATETREE_POD_INSTANCEDATA(FZCCompareActorDistanceConditionInstanceData);

USTRUCT(DisplayName = "ZC액터 거리 비교", Category = "Zelda")
struct FZCCompareActorDistanceCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCCompareActorDistanceConditionInstanceData;

	FZCCompareActorDistanceCondition() = default;

	explicit FZCCompareActorDistanceCondition(const EGenericAICheck InOperator, const EStateTreeCompare InInverts = EStateTreeCompare::Default)
		: bInvert(InInverts == EStateTreeCompare::Invert)
		, Operator(InOperator)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (InvalidEnumValues = "IsTrue"))
	EGenericAICheck Operator = EGenericAICheck::Equal;
};