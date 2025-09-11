// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/Evaluation/ZCAIPerceptionSTE.h"
#include "Component/Perception/ZCAIPerceptionComponent.h"
#include "Actor/Item/Weapon/ZCWeaponActor.h"
#include "Actor/Item/Shield/ZCShieldActor.h"

void FZCAIPerceptionSTE::TreeStart(FStateTreeExecutionContext& Context) const
{
	Super::TreeStart(Context);

	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.AIController)
	{
		InstanceData.AIPerceptionComponent = InstanceData.AIController->GetZCPerceptionComponent();
		InstanceData.AggroThreshold = InstanceData.AIPerceptionComponent->GetAggroThreshold();
		InstanceData.AggroDecrementDelay = InstanceData.AIPerceptionComponent->GetAggroDecrementDelay();
		InstanceData.AggroResetTime = InstanceData.AIPerceptionComponent->GetAggroResetTime();
		InstanceData.SenseDetectionInfo = InstanceData.AIPerceptionComponent->GetPerceptionData();
		InstanceData.bIsPerception = InstanceData.SenseDetectionInfo.SightInfo.bIsDetected | InstanceData.SenseDetectionInfo.HearingInfo.bIsDetected | InstanceData.SenseDetectionInfo.DamageInfo.bIsDetected;
		InstanceData.CurrentAggro = InstanceData.AIPerceptionComponent->GetCurrentAggro();
		InstanceData.TimeSinceLastPerception = 0.0f;
		InstanceData.CurrentWeapon = InstanceData.MonsterCharacter->GetCurrentWeapon();
		InstanceData.CurrentShield = InstanceData.MonsterCharacter->GetCurrentShield();
	}
}

void FZCAIPerceptionSTE::TreeStop(FStateTreeExecutionContext& Context) const
{
	Super::TreeStop(Context);

	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.bIsPerception = false;
	InstanceData.TimeSinceLastPerception = 0.0f;
}

void FZCAIPerceptionSTE::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	Super::Tick(Context, DeltaTime);
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.AIController)
	{
		InstanceData.SenseDetectionInfo = InstanceData.AIPerceptionComponent->GetPerceptionData();
		InstanceData.bIsPerception = InstanceData.SenseDetectionInfo.SightInfo.bIsDetected | InstanceData.SenseDetectionInfo.HearingInfo.bIsDetected | InstanceData.SenseDetectionInfo.DamageInfo.bIsDetected;
		InstanceData.CurrentAggro = InstanceData.AIPerceptionComponent->GetCurrentAggro();
		!InstanceData.bIsPerception ? InstanceData.TimeSinceLastPerception += DeltaTime : InstanceData.TimeSinceLastPerception = 0.0f;
		InstanceData.CurrentWeapon = InstanceData.MonsterCharacter->GetCurrentWeapon();
		InstanceData.CurrentShield = InstanceData.MonsterCharacter->GetCurrentShield();
	}
}

FText FZCAIPerceptionSTE::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("ZC 퍼셉션 감각 평가"));
}

FName FZCAIPerceptionSTE::GetIconName() const
{
	return FName("StateTree.Evaluator");
}

FColor FZCAIPerceptionSTE::GetIconColor() const
{
	return FColor::Orange; // 퍼셉션을 나타내는 주황색
}
