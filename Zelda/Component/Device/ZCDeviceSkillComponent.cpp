// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Device/ZCDeviceSkillComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "Player/ZCPlayerController.h"

#include "Player/ZCHUDLocalPlayerSubsystem.h"

#include "Actor/ZCActor.h"
#include "ZCDeviceEnum.h"

#include "Development/ZCLogger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCDeviceSkillComponent)

// Sets default values for this component's properties
UZCDeviceSkillComponent::UZCDeviceSkillComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = false;
	// ...
}

void UZCDeviceSkillComponent::SetupInputComponent(UEnhancedInputComponent* InputComponent)
{
	InputComponent->BindAction(SelectAction, ETriggerEvent::Triggered, this, &UZCDeviceSkillComponent::Action);
	InputComponent->BindAction(CancelAction, ETriggerEvent::Triggered, this, &UZCDeviceSkillComponent::Cancel);
}

void UZCDeviceSkillComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
}

void UZCDeviceSkillComponent::Deactivate()
{
	Super::Deactivate();
	OnDeactivate.Broadcast(SkillType);
}

bool UZCDeviceSkillComponent::ShouldActivate() const
{
	const bool bSuperActive = Super::ShouldActivate();
	return bSuperActive;
}

void UZCDeviceSkillComponent::Action()
{
}

void UZCDeviceSkillComponent::Cancel()
{
}

bool UZCDeviceSkillComponent::GetCameraRayToCharacterRay(FVector& OutStart, FVector& OutEnd, float Distance) const
{
	FRotator ViewRot;

	if (!PC) return false;
	PC->GetPlayerViewPoint(OutStart, ViewRot);

	ACharacter* PlayerPawn = PC->GetCharacter();
	if (!PlayerPawn) return false;

	FVector PawnLocation = PlayerPawn->GetActorLocation();
	PawnLocation.Z += PlayerPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector Direction = PawnLocation - OutStart;
	Direction.Normalize();

	OutStart = PawnLocation;
	OutEnd = OutStart + (Direction * Distance);
	return true;
}

bool UZCDeviceSkillComponent::DeviceOverlapMultiByChannel(TArray<FOverlapResult>& OutOverlapResults, const FVector& Pos, const FQuat& Rot, const FCollisionShape& Shape) const
{
	OutOverlapResults.Reset();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DeviceOverlapMultiByChannel), false);
	QueryParams.AddIgnoredActor(PC);
	QueryParams.AddIgnoredActor(PC->GetPawn());

	return GetWorld()->OverlapMultiByChannel(OutOverlapResults, Pos, Rot, DeviceSkillCollisionChannel, Shape, QueryParams);
}

bool UZCDeviceSkillComponent::DeviceSweepSingleByChannel(struct FHitResult& OutHit, const FVector& Start, const FVector& End, const FQuat& Rot, const FCollisionShape& Shape) const
{
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DeviceSweepSingleByChannel), false);
	QueryParams.AddIgnoredActor(PC);
	QueryParams.AddIgnoredActor(PC->GetPawn());

	return GetWorld()->SweepSingleByChannel(OutHit, Start, End, Rot, DeviceSkillCollisionChannel, Shape, QueryParams);
}

template<typename ContainerType>
AZCActor* UZCDeviceSkillComponent::GetNearestActorFromPlayer_Internal(const ContainerType& Actors) const
{
	if (!PC) return nullptr;

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return nullptr;

	const FVector PawnLocation = PlayerPawn->GetActorLocation();

	return FindNearestActorAtLocation_Internal(PawnLocation, Actors);
}

template<typename ContainerType>
AZCActor* UZCDeviceSkillComponent::FindNearestActorAtLocation_Internal(const FVector& Location, const ContainerType& Actors) const
{
	float BestDistSq = MAX_flt;
	AZCActor* BestActor = nullptr;

	for (AZCActor* Actor : Actors)
	{
		if (!IsValid(Actor)) continue;

		const float DistSq = FVector::DistSquared(Location, Actor->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestActor = Actor;
		}
	}

	return BestActor;
}

AZCActor* UZCDeviceSkillComponent::GetNearestActorFromPlayer(const TArray<AZCActor*>& Actors) const
{
	return GetNearestActorFromPlayer_Internal(Actors);
}

AZCActor* UZCDeviceSkillComponent::GetNearestActorFromPlayer(const TSet<AZCActor*>& Actors) const
{
	return GetNearestActorFromPlayer_Internal(Actors);
}

AZCActor* UZCDeviceSkillComponent::GetNearestActorAtLocation(const FVector& Location, const TArray<AZCActor*>& Actors) const
{
	return FindNearestActorAtLocation_Internal(Location, Actors);
}

AZCActor* UZCDeviceSkillComponent::GetNearestActorAtLocation(const FVector& Location, const TSet<AZCActor*>& Actors) const
{
	return FindNearestActorAtLocation_Internal(Location, Actors);
}
