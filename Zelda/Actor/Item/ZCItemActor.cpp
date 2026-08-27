// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Item/ZCItemActor.h"
#include "Components/SphereComponent.h"
#include "Player/ZCPlayerController.h"
#include "Character/ZCCharacter.h"
#include "GameData/PrimaryData/Item/ZCItemPrimaryData.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Development/ZCLogger.h"
#include "Component/VFX/ZCNiagaraComponent.h"
#include "Component/Chemistry/ZCMaterialStateComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCItemActor)

AZCItemActor::AZCItemActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<USphereComponent>(AZCActor::InteractionAreaName))
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh->SetCollisionProfileName(Zelda::Profile::Item);
	Mesh->SetVisibility(true);

	NiagaraComponent->SetupAttachment(RootComponent);

	SphereInteractionArea = Cast<USphereComponent>(InteractionArea);
	SphereInteractionArea->InitSphereRadius(200.0f);
	SphereInteractionArea->SetupAttachment(Mesh);
}

TUniquePtr<FZCItemTable> AZCItemActor::OnItemPickUp()
{
	TUniquePtr<FZCItemTable> Item = GetWorld()->GetSubsystem<UZCWorldSubsystem>()->TakeItemFromPool(ItemInfo->ItemType, ItemIndex);
	Destroy();

	return TUniquePtr<FZCItemTable>(Item.Release());
}

void AZCItemActor::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if (!NewOwner)
	{
		Mesh->SetCollisionProfileName(Zelda::Profile::Item);
	}
}

void AZCItemActor::Initialize(FZCActorTable* NewInfo)
{
	Super::Initialize(NewInfo);

	if (NewInfo->Type != EZCActortype::Item) return;

	ItemInfo = static_cast<FZCItemTable*>(NewInfo);
}

void AZCItemActor::OnEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnEnterRange(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (AZCPlayerController* PC = Cast<AZCPlayerController>(Pawn->GetController()))
		{
			PC->NotifyItemInRange(this);
		}
	}
}

void AZCItemActor::OnExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnExitRange(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (AZCPlayerController* PC = Cast<AZCPlayerController>(Pawn->GetController()))
		{
			PC->NotifyItemOutRange(this);
		}
	}
}

