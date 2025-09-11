// Copyright Epic Games, Inc. All Rights Reserved.


#include "ZCStateTreeUtility.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeExecutionTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeLinker.h"
#include "Tasks/AITask_MoveTo.h"

#include "Character/Monster/ZCMonsterCharacter.h"
#include "AI/ZCAIControllerBase.h"

#include "Component/Perception/ZCAIPerceptionComponent.h"

#include "Actor/Spline/ZCPatrolSplineActor.h"


// ZC이동 함수
EStateTreeRunStatus FZCStateTreeMoveToTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AIController) 	return EStateTreeRunStatus::Failed;

	InstanceData.TaskOwner = TScriptInterface<IGameplayTaskOwnerInterface>(InstanceData.AIController->FindComponentByInterface(UGameplayTaskOwnerInterface::StaticClass()));
	if (!InstanceData.TaskOwner)
	{
		InstanceData.TaskOwner = InstanceData.AIController;
	}

	return PerformMoveTask(Context, *InstanceData.AIController);
}

EStateTreeRunStatus FZCStateTreeMoveToTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.MoveToTask)
	{
		if (InstanceData.MoveToTask->GetState() == EGameplayTaskState::Finished)
		{
			return InstanceData.MoveToTask->WasMoveSuccessful() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
		}

		if (InstanceData.bTrackMovingGoal)
		{
			const FVector CurrentDestination = InstanceData.MoveToTask->GetMoveRequestRef().GetDestination();
			if (FVector::DistSquared(CurrentDestination, InstanceData.Destination) > (InstanceData.DestinationMoveTolerance * InstanceData.DestinationMoveTolerance))
			{
				return PerformMoveTask(Context, *InstanceData.AIController);
			}
		}
		return EStateTreeRunStatus::Running;
	}
	return EStateTreeRunStatus::Failed;
}

void FZCStateTreeMoveToTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.MoveToTask && InstanceData.MoveToTask->GetState() != EGameplayTaskState::Finished)
	{
		InstanceData.MoveToTask->ExternalCancel();
	}
}

UAITask_MoveTo* FZCStateTreeMoveToTask::PrepareMoveToTask(FStateTreeExecutionContext& Context, AZCAIControllerBase& Controller, UAITask_MoveTo* ExistingTask, FAIMoveRequest& MoveRequest) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UAITask_MoveTo* MoveTask = ExistingTask ? ExistingTask : UAITask::NewAITask<UAITask_MoveTo>(Controller, *InstanceData.TaskOwner);
	if (MoveTask)
	{
		MoveTask->SetUp(&Controller, MoveRequest);
	}

	return MoveTask;
}

EStateTreeRunStatus FZCStateTreeMoveToTask::PerformMoveTask(FStateTreeExecutionContext& Context, AZCAIControllerBase& Controller) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FAIMoveRequest MoveReq;
	MoveReq.SetAllowPartialPath(InstanceData.bAllowPartialPath)
		.SetAcceptanceRadius(InstanceData.AcceptableRadius)
		.SetCanStrafe(InstanceData.bAllowStrafe)
		.SetReachTestIncludesAgentRadius(InstanceData.bReachTestIncludesAgentRadius)
		.SetReachTestIncludesGoalRadius(InstanceData.bReachTestIncludesGoalRadius)
		.SetRequireNavigableEndLocation(InstanceData.bRequireNavigableEndLocation)
		.SetProjectGoalLocation(InstanceData.bProjectGoalLocation)
		.SetUsePathfinding(true);

	InstanceData.GoalActor != nullptr ? MoveReq.SetGoalActor(InstanceData.GoalActor) : MoveReq.SetGoalLocation(InstanceData.Destination);

	if (MoveReq.IsValid())
	{
		InstanceData.MoveToTask = PrepareMoveToTask(Context, Controller, InstanceData.MoveToTask, MoveReq);
		if (InstanceData.MoveToTask)
		{
			if (InstanceData.MoveToTask->IsActive())
			{
				InstanceData.MoveToTask->ConditionalPerformMove();
			}
			else
			{
				InstanceData.MoveToTask->ReadyForActivation();
			}

			if (InstanceData.MoveToTask->GetState() == EGameplayTaskState::Finished)
			{
				return InstanceData.MoveToTask->WasMoveSuccessful() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
			}

			return EStateTreeRunStatus::Running;
		}
	}
	return EStateTreeRunStatus::Failed;
}

#if WITH_EDITOR
FText FZCStateTreeMoveToTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 목표 지점으로 이동"));
}
#endif // WITH_EDITOR


////////////////////////////////////////////////////////////////////////////////////////
// ZC 어그로 리셋 함수
EStateTreeRunStatus FZCStateTreeResetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AIController) 	return EStateTreeRunStatus::Failed;

	InstanceData.AIController->GetZCPerceptionComponent()->ResetAggro();

	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FZCStateTreeResetTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 어그로 리셋"));
}
#endif // WITH_EDITOR


////////////////////////////////////////////////////////////////////////////////////////
// ZC 스플라인 찾기 함수
EStateTreeRunStatus FZCStateTreeFindSplineTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.AIController || !InstanceData.MonsterCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector AIPosition = InstanceData.MonsterCharacter->GetActorLocation();

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::StateTree;
	if (bDrawDebugAll || bDrawTask)
	{
		DrawDebugSphere(InstanceData.AIController->GetWorld(), AIPosition, InstanceData.SplineSearchRadius, 16, FColor::Red, false, 1.0f, 0, 2.0f);
	}
#endif // !UE_BUILD_SHIPPING


	return EStateTreeRunStatus();
}

FText FZCStateTreeFindSplineTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 스플라인 찾기"));
}


////////////////////////////////////////////////////////////////////////////////////////
// ZC 스플라인 따라가기 함수
EStateTreeRunStatus FZCStateTreeFollowSplineTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.AIController || !InstanceData.MonsterCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector AIPosition = InstanceData.MonsterCharacter->GetActorLocation();

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::StateTree;
	if (bDrawDebugAll || bDrawTask)
	{
		DrawDebugSphere(InstanceData.AIController->GetWorld(), AIPosition, InstanceData.SplineSearchRadius, 16, FColor::Red, false, 1.0f, 0, 2.0f);
	}
#endif // !UE_BUILD_SHIPPING

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel6));

	TArray<AActor*> FoundSplines;
	UKismetSystemLibrary::SphereOverlapActors(InstanceData.AIController, AIPosition, InstanceData.SplineSearchRadius, ObjectTypes, AZCPatrolSplineActor::StaticClass(), {}, FoundSplines);

	if (FoundSplines.Num() == 0) return EStateTreeRunStatus::Failed;

	float NearestDistance = 0.0f;
	AActor* NearestSpline = UGameplayStatics::FindNearestActor(AIPosition, FoundSplines, NearestDistance);
	AZCPatrolSplineActor* PatrolSpline = Cast<AZCPatrolSplineActor>(NearestSpline);
	if (!PatrolSpline) return EStateTreeRunStatus::Failed;


#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::StateTree;

	if (bDrawDebugAll || bDrawTask)
	{
		DrawDebugLine(InstanceData.AIController->GetWorld(), AIPosition, PatrolSpline->GetActorLocation(), FColor::Green, false, 1.0f, 0, 2.0f);
	}
#endif // !UE_BUILD_SHIPPING


	InstanceData.CurrentSplineComponent = PatrolSpline->GetSplineComponent();
	InstanceData.SplineLength = InstanceData.CurrentSplineComponent->GetSplineLength();
	InstanceData.CurrentSplineDistance = InstanceData.CurrentSplineComponent->GetDistanceAlongSplineAtLocation(AIPosition, ESplineCoordinateSpace::Type::World);
	InstanceData.bIsMoving = true;
	InstanceData.Timer = 0.0f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FZCStateTreeFollowSplineTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AIController || !InstanceData.CurrentSplineComponent || !InstanceData.MonsterCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Timer += DeltaTime;

	if (InstanceData.bIsMoving)
	{
		if (InstanceData.Timer >= InstanceData.PatrolMoveTime)
		{
			InstanceData.bIsMoving = false;
			InstanceData.Timer = 0.0f;
			InstanceData.AIController->StopMovement();
			return EStateTreeRunStatus::Running;
		}

		InstanceData.CurrentSplineDistance += InstanceData.SplineMoveSpeed * DeltaTime;

		if (InstanceData.CurrentSplineDistance > InstanceData.SplineLength)
		{
			InstanceData.CurrentSplineDistance = FMath::Fmod(InstanceData.CurrentSplineDistance, InstanceData.SplineLength);
		}

		const FVector TargetLocation = InstanceData.CurrentSplineComponent->GetLocationAtDistanceAlongSpline(InstanceData.CurrentSplineDistance, ESplineCoordinateSpace::World);
		InstanceData.AIController->MoveToLocation(TargetLocation, InstanceData.SplinePathTolerance);
	}
	else
	{
		if (InstanceData.Timer >= InstanceData.PatrolStopTime)
		{
			InstanceData.bIsMoving = true;
			InstanceData.Timer = 0.0f;

			const FVector RetryTargetLocation = InstanceData.CurrentSplineComponent->GetLocationAtDistanceAlongSpline(InstanceData.CurrentSplineDistance, ESplineCoordinateSpace::World);

			InstanceData.AIController->MoveToLocation(RetryTargetLocation, InstanceData.SplinePathTolerance);
		}
	}

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::StateTree;
	if (bDrawDebugAll || bDrawTask)
	{
		const FString DebugText = FString::Printf(TEXT("%s | %.2f / %.2f"), InstanceData.bIsMoving ? TEXT("이동 중") : TEXT("정지 중"), InstanceData.Timer, InstanceData.bIsMoving ? InstanceData.PatrolMoveTime : InstanceData.PatrolStopTime);

		DrawDebugString(InstanceData.AIController->GetWorld(), InstanceData.MonsterCharacter->GetActorLocation() + FVector(0, 0, 200), DebugText, nullptr, FColor::Green, 0.0f, true);
	}

#endif // !UE_BUILD_SHIPPING


	return EStateTreeRunStatus::Running;
}

void FZCStateTreeFollowSplineTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.AIController)
	{
		InstanceData.AIController->StopMovement();
	}
}

#if WITH_EDITOR
FText FZCStateTreeFollowSplineTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 스플라인 따라가기"));
}
#endif // WITH_EDITOR

