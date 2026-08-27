// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Item/Gear/ZCGearActor.h"
#include "Components/SphereComponent.h"

#include "Gameplay/GameplayTag/ZCGameplayTag.h"

#include "Zelda/Physics/ZCCollision.h"
#include "ZeldaPhysics/Public/Runtime/Gear/ZCGearMovementComponent.h"
#include "ZeldaPhysics/Public/Runtime/Gear/ZCGearFanComponent.h"
#include "ZeldaPhysics/Public/Runtime/Gear/ZCGearBalloonComponent.h"

AZCGearActor::AZCGearActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<USphereComponent>(AZCActor::InteractionAreaName))
{
	CanShowOverlayState &= ~static_cast<uint8>(EOverlayState::ItemHighlight);

	SphereInteractionArea = Cast<USphereComponent>(InteractionArea);
	SphereInteractionArea->InitSphereRadius(200.0f);
	//SphereInteractionArea->SetupAttachment(RootComponent);

	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionProfileName(Zelda::Profile::Gear);

	GearMovementComponent = CreateDefaultSubobject<UZCGearMovementComponent>(TEXT("GearMovementComponent"));
}

void AZCGearActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GearMovementComponent->CreateGearModules();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AZCGearFanActor::AZCGearFanActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UZCGearFanComponent>(AZCActor::MeshComponentName))
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AZCGearBalloonActor::AZCGearBalloonActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UZCGearBalloonComponent>(AZCActor::MeshComponentName))
{
}

void AZCGearBalloonActor::GetElement(const FGameplayTag& ElementTag)
{
	//TAG_Element_Fire
	if (ElementTag == TAG_Element_Fire)
	{

		FTimerManager& TimerManager = GetWorld()->GetTimerManager();

		if (TimerManager.IsTimerActive(BalloonPopTimerHandle) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("타이머가 켜짐"));
			
			GearMovementComponent->EnableSimulation();

		}
		TimerManager.SetTimer(BalloonPopTimerHandle, this, &AZCGearBalloonActor::OnFireTimeOut, 1.0f, false);
	}
}

void AZCGearBalloonActor::OnFireTimeOut()
{
	UE_LOG(LogTemp, Warning, TEXT("불이 없음"));
	
	GearMovementComponent->DisableSimulation();

	GetWorld()->GetTimerManager().ClearTimer(BalloonPopTimerHandle);
}
