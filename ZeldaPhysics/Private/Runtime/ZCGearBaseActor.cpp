// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/ZCGearBaseActor.h"
#include "Runtime/Gear/ZCGearMovementComponent.h"

// Sets default values
AZCGearBaseActor::AZCGearBaseActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetSimulatePhysics(true);

	RootComponent = StaticMesh;

	GearMovementComponent = CreateDefaultSubobject<UZCGearMovementComponent>(TEXT("GearMovementComponent"));
	
	GearMovementComponent->bAutoRegisterUpdatedComponent = true;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicatingMovement(true);
}

void AZCGearBaseActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	GearMovementComponent->CreateGearModules();
}
