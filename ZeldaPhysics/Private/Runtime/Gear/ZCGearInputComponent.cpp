// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/Gear/ZCGearInputComponent.h"
#include "Runtime/Gear/ZCGearMovementComponent.h"
#include "Core/Module/ZCSimGearInputModule.h"
#include "Public/ZCSimGearManager.h"

UZCGearInputComponent::UZCGearInputComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

ZCGear::IZCSimGearBaseModule* UZCGearInputComponent::CreateNewGearModule() const
{
	ZCGear::FZCInputSettings Settings;

	ZCGear::IZCSimGearBaseModule* NewModule = new ZCGear::FZCSimGearInputModule(Settings);
	return NewModule;
}

void UZCGearInputComponent::StartRide(const TArray<UZCGearMovementComponent*>& ConnectedGearMovements)
{
	FindGearManager();

	for (UZCGearMovementComponent* GearMovement : ConnectedGearMovements)
	{
		GearManager->RegisterPlayerInputGearSimulation(GearMovement);
	}

	GearManager->SetPlayerViewVector(CalculatePlayerViewLocalVector());
}

void UZCGearInputComponent::EndRide(const TArray<UZCGearMovementComponent*>& ConnectedGearMovements)
{
	FindGearManager();

	for (UZCGearMovementComponent* GearMovement : ConnectedGearMovements)
	{
		GearManager->UnRegisterPlayerInputGearSimulation(GearMovement);
	}
}

void UZCGearInputComponent::ApplyPlayerInput(const FVector2D& RawInput)
{
	if (GearManager)
	{
		GearManager->PlayerInput(RawInput);
	}
}

void UZCGearInputComponent::SetCenterOfMassOffset(const FVector& InCenterOfMassOffset, const float InMass)
{
	CenterOfMassOffset = InCenterOfMassOffset;
	TotalMass = InMass;

	FindGearManager();
	if (GearManager)
	{
		GearManager->SetCenterOfMassOffset(CenterOfMassOffset, TotalMass);
	}
}

void UZCGearInputComponent::FindGearManager()
{
	if (UWorld* World = GetWorld())
	{
		if (FPhysScene* PhysScene = World->GetPhysicsScene())
		{
			GearManager = FZCSimGearManager::GetManagerFromScene(PhysScene);
		}
	}
}

FVector UZCGearInputComponent::CalculatePlayerViewLocalVector() const
{
	FVector WorldViewVector = GetOwner()->GetActorRotation().Vector();
	FVector LocalViewVector = GetOwner()->GetActorQuat().UnrotateVector(WorldViewVector);
	return LocalViewVector;
}
