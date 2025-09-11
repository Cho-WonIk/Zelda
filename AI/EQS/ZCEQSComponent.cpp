// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/ZCEQSComponent.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"

// Sets default values for this component's properties
UZCEQSComponent::UZCEQSComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UZCEQSComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UZCEQSComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UZCEQSComponent::RunEQSQuery()
{
	if (!EQSQuery || !GetWorld()) return;

	UObject* QueryOwner = GetOwner();

	QueryResultWrapper = UEnvQueryManager::RunEQSQuery(GetWorld(), EQSQuery, QueryOwner, RunMode, nullptr);

	if (QueryResultWrapper)
	{
		QueryResultWrapper->GetOnQueryFinishedEvent().AddDynamic(this, &UZCEQSComponent::OnQueryFinished);
	}
}

FVector UZCEQSComponent::GetBestLocation() const
{
	return CachedLocations.Num() > 0 ? CachedLocations[0] : FVector::ZeroVector;
}

AActor* UZCEQSComponent::GetBestActor() const
{
	return CachedActors.Num() > 0 ? CachedActors[0] : nullptr;
}

void UZCEQSComponent::OnQueryFinished(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	if (QueryStatus == EEnvQueryStatus::Success)
	{
		CachedLocations.Reset();
		CachedActors.Reset();

		QueryInstance->GetQueryResultsAsLocations(CachedLocations);
		QueryInstance->GetQueryResultsAsActors(CachedActors);
	}
}


