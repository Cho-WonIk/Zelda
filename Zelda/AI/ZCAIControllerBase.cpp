// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ZCAIControllerBase.h"

#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"

#include "Components/StateTreeAIComponent.h"
#include "Component/Perception/ZCAIPerceptionComponent.h"
#include "Component/PathFollowing/ZCPathFollowingComponent.h"
#include "Interface/ZCAIInterface.h"
#include "Utils/Team/ZCTeam.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCAIControllerBase)

AZCAIControllerBase::AZCAIControllerBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UZCPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("State Tree AI Component"));
	StateTreeAIComponent->SetStartLogicAutomatically(true);

	PerceptionComponent = CreateDefaultSubobject<UZCAIPerceptionComponent>(TEXT("Perception Component"));

	ZCPerceptionComponent = ExactCast<UZCAIPerceptionComponent>(PerceptionComponent);

	ZCPathFollowingComponent = ExactCast<UZCPathFollowingComponent>(GetPathFollowingComponent());

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AZCAIControllerBase::TargetPerceptionUpdated);
}

ETeamAttitude::Type AZCAIControllerBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other);
	if (!OtherTeamAgent)
	{
		return ETeamAttitude::Neutral;
	}

	const FGenericTeamId ThisTemaId = GetGenericTeamId();
	const FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();

	return ZCTeamUtils::GetTeamAttitude(ThisTemaId, OtherTeamId);
}

void AZCAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IGenericTeamAgentInterface* OwningPawn = Cast<IGenericTeamAgentInterface>(InPawn))
	{
		SetGenericTeamId(OwningPawn->GetGenericTeamId());
	}
}

void AZCAIControllerBase::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{

}