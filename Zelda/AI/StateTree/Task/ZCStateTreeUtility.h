// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Tasks/StateTreeAITask.h"
#include "Templates/SubclassOf.h"

#include "EnvironmentQuery/EnvQueryTypes.h"
#include "NativeGameplayTags.h"
#include "StateTreePropertyRef.h"

#include "ZCStateTreeInstanceBase.h"

#include "ZCStateTreeUtility.generated.h"

class AZCMonsterCharacter;
class AZCAIControllerBase;

class IGameplayTaskOwnerInterface;
class UAITask_MoveTo;

class USplineComponent;

struct FAIMoveRequest;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCMoveToTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "목표 위치"))
	FVector Destination = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "목표 액터"))
	AActor* GoalActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "도달로 판정하는 범위"))
	float AcceptableRadius = 100.0f;

	// 완벽히 도달하지 못해도 부분 도달을 허용할지 여부
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "부분 도달 가능 여부"))
	bool bAllowPartialPath = true;

	// 목적지로 이동하는 도중 목적지가 변경 시 추적 여부
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "목적지 변경 추적 여부"))
	bool bTrackMovingGoal = true;

	// 목적지로 이동하는 도중 목적지가 변경 시, 얼마나 민감하게 반응할지 조정
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bTrackMovingGoal", DisplayName = "목적지 도달 민감도"))
	float DestinationMoveTolerance = 0.f;

	// true : 시선은 플레이어를 고정, false : 이동방향과 시야가 일치
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "이동 방향과 시야 일치 여부"))
	bool bAllowStrafe = false;

	// 목표지점이 항상 NavMesh위에 존재해야하는지
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "목표지점이 NavMesh 위에 존재해야함"))
	bool bRequireNavigableEndLocation = true;

	// 목표지점이 NavMesh 위에 존재하지 않을 경우, 목표지점을 NavMesh 위로 투사할지 여부
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "목표지점 NavMesh 위로 투사 여부"))
	bool bProjectGoalLocation = true;

	// AI캡슐 반지름이 도달 판정 거리 추가 여부
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "AI 캡슐 반지름 포함 여부"))
	bool bReachTestIncludesAgentRadius = true;

	// 반지름이 도달 판정 거리에 포함 여부
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "목표지점 반지름 포함 여부"))
	bool bReachTestIncludesGoalRadius = true;

	UPROPERTY(Transient)
	TObjectPtr<UAITask_MoveTo> MoveToTask = nullptr;

	UPROPERTY(Transient)
	TScriptInterface<IGameplayTaskOwnerInterface> TaskOwner = nullptr;
};

USTRUCT(meta = (DisplayName = "ZC 이동 함수", Category = "Zelda|Action"))
struct FZCStateTreeMoveToTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCMoveToTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual UAITask_MoveTo* PrepareMoveToTask(FStateTreeExecutionContext& Context, AZCAIControllerBase& Controller, UAITask_MoveTo* ExistingTask, FAIMoveRequest& MoveRequest) const;
	virtual EStateTreeRunStatus PerformMoveTask(FStateTreeExecutionContext& Context, AZCAIControllerBase& Controller) const;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FName GetIconName() const override { return FName("StateTreeEditorStyle|Node.Movement"); }
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR

};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCResetTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()
};

USTRUCT(meta = (DisplayName = "ZC 어그로 리셋", Category = "Zelda|Util"))
struct FZCStateTreeResetTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCResetTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCFindSplineTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()
	// 스플라인 탐색 범위
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "스플라인 탐색 범위"))
	float SplineSearchRadius = 2000.0f;
	// 스플라인 검색 결과
	UPROPERTY(VisibleAnywhere, Category = "Out", meta = (RefType = "/Script/Engine.SplineComponent"))
	FStateTreePropertyRef SplineActorProperty;
};


USTRUCT(meta = (DisplayName = "ZC 스플라인 검색", Category = "Zelda|Util"))
struct FZCStateTreeFindSplineTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCFindSplineTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT()
struct FZCFollowSplineTaskInstanceData : public FZCStandardInstanceData
{
	GENERATED_BODY()

	// 스플라인 탐색 범위
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "스플라인 탐색 범위"))
	float SplineSearchRadius = 2000.0f;

	// 스플라인 경로 오차 범위
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "스플라인 경로 오차 범위"))
	float SplinePathTolerance = 5.0f;

	// 스플라인 경로를 따라 이동하는 속도
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "스플라인 경로 이동 속도"))
	float SplineMoveSpeed = 300.0f;

	// 정찰 중 이동하는 시간 간격
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "정찰 중 이동 시간 간격"))
	float PatrolMoveTime = 1.0f;

	// 정찰 중 정지하는 시간 간격
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "정찰 중 정지 시간 간격"))
	float PatrolStopTime = 1.0f;

	// 현재 정지 중인지 아닌지 여부, true 이면 이동 중, false 이면 정지 중
	UPROPERTY(Transient, EditAnywhere, Category = "Output", meta = (DisplayName = "현재 정지 중인지 여부"))
	bool bIsMoving = false;

	float CurrentSplineDistance = 0.0f;
	float SplineLength = 0.0f;
	float Timer = 0.0f;

	USplineComponent* CurrentSplineComponent = nullptr;
};

USTRUCT(meta = (DisplayName = "ZC 스플라인 따라가기", Category = "Zelda|Action"))
struct FZCStateTreeFollowSplineTask : public FStateTreeAITaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCFollowSplineTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::Blue; }
#endif // WITH_EDITOR
};

