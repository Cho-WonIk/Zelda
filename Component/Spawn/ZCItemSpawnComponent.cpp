// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Spawn/ZCItemSpawnComponent.h"
#include "World/Subsystem/ZCWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCItemSpawnComponent)

// Sets default values for this component's properties
UZCItemSpawnComponent::UZCItemSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UZCItemSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnItem();
	}
	
}

void UZCItemSpawnComponent::SpawnItem()
{
	if (IsValid(ItemClass))
	{
		GetWorld()->GetSubsystem<UZCWorldSubsystem>()->SpawnItemByID(ItemType, ItemID, ItemClass, GetComponentTransform());
	}
}

