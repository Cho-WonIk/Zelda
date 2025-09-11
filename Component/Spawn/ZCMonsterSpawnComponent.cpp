// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Spawn/ZCMonsterSpawnComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UZCMonsterSpawnComponent::UZCMonsterSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(FName("NoCollision"));

	SpawnDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn Direction"));
	SpawnDirection->SetupAttachment(this);
}

void UZCMonsterSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnMonster();
	}
}

void UZCMonsterSpawnComponent::SpawnMonster()
{
	if (IsValid(MonsterClass))
	{
		AZCMonsterCharacter* SpawnedMonster = GetWorld()->SpawnActorDeferred<AZCMonsterCharacter>(MonsterClass, GetComponentTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (!SpawnedMonster) return;

		SpawnedMonster->SetIdleType(IdleType);
		if (IdleType == EIdleType::Talk) SpawnedMonster->SetPlacMonsterOnTalk(PlaceMonsterOnTalk);

		UGameplayStatics::FinishSpawningActor(SpawnedMonster, GetComponentTransform());
	}
}

