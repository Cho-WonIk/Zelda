// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControl/ZCMonsterGroupActor.h"
#include "Components/SphereComponent.h"
#include "AI/EQS/ZCEQSComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCMonsterGroupActor)

// Sets default values
AZCMonsterGroupActor::AZCMonsterGroupActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MonsterArea = CreateDefaultSubobject<USphereComponent>(TEXT("MonsterArea"));
	MonsterArea->SetSphereRadius(MonsterRadius);

	RootComponent = MonsterArea;

	EQSComponent = CreateDefaultSubobject<UZCEQSComponent>(TEXT("EQSComponent"));
}

// Called when the game starts or when spawned
void AZCMonsterGroupActor::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void AZCMonsterGroupActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AZCMonsterGroupActor, MonsterRadius))
	{
		if (MonsterArea)
		{
			MonsterArea->SetSphereRadius(MonsterRadius);
		}
	}
}
#endif // WITH_EDITOR

void AZCMonsterGroupActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (MonsterArea)
	{
		MonsterArea->SetSphereRadius(MonsterRadius);
	}
}
