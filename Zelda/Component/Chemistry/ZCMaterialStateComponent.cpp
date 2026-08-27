// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Chemistry/ZCMaterialStateComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameData/Struct/ZCChemistryStruct.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Actor/ZCActor.h"
#include "Development/ZCLogger.h"
#include "Component/VFX/ZCNiagaraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCMaterialStateComponent)

UZCMaterialStateComponent::UZCMaterialStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;
}

void UZCMaterialStateComponent::BeginPlay()
{
	Super::BeginPlay();

	WorldSubsystem = GetWorld()->GetSubsystem<UZCWorldSubsystem>();

	if (WorldSubsystem && OwnerCasted && VFXComponentCached)
	{
		// Subsystem에 등록하고 핸들 획득
		ChemistryHandle = WorldSubsystem->RegisterObject(OwnerCasted, VFXComponentCached, Material);
	}
}

void UZCMaterialStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Subsystem에서 등록 해제
	if (WorldSubsystem && ChemistryHandle.IsValid())
	{
		WorldSubsystem->UnregisterObject(ChemistryHandle);
		ChemistryHandle = FZCChemistryHandle();
	}

	WorldSubsystem = nullptr;
	VFXComponentCached = nullptr;
	OwnerCasted = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UZCMaterialStateComponent::NotifyTakeDamageExposure(const FElementInfo& NewElementInfo)
{
	if (WorldSubsystem && ChemistryHandle.IsValid())
	{
		WorldSubsystem->NotifyTakeDamageExposure(ChemistryHandle, NewElementInfo);
	}
}

void UZCMaterialStateComponent::NotifyHit(const FVector& Location, const FVector& Direction)
{
	if (WorldSubsystem && ChemistryHandle.IsValid())
	{
		WorldSubsystem->NotifyHit(ChemistryHandle, Location, Direction);
	}
}

bool UZCMaterialStateComponent::HasActiveElement() const
{
	if (const FZCRuntimeActorChemistryState* State = WorldSubsystem->GetRuntimeData(ChemistryHandle))
	{
		return State->IsActive();
	}
	return false;
}

FGameplayTag UZCMaterialStateComponent::GetCurrentElementTag() const
{
	if (const FZCRuntimeActorChemistryState* State = WorldSubsystem->GetRuntimeData(ChemistryHandle))
	{
		return State->CurrentElementTag;
	}
	return FGameplayTag::EmptyTag;
}

#if WITH_EDITOR
void UZCMaterialStateComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UZCMaterialStateComponent, DebugElementTag))
	{
		if (WorldSubsystem && ChemistryHandle.IsValid())
		{
			FZCReactionOut ForcedOut;
			ForcedOut.ReactionElementTag = DebugElementTag;
			ForcedOut.Duration = -1.0f;       // 무한 지속
			ForcedOut.bIsDamageOnce = false;
			ForcedOut.FirstDamage = 10.0f;
			ForcedOut.TickDamage = 5.0f;
			ForcedOut.TickInterval = 1.0f;

			WorldSubsystem->DebugForceElement(ChemistryHandle, ForcedOut);
		}
	}
}
#endif //WITH_EDITOR

