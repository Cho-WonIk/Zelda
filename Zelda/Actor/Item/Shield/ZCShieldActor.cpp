// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Item/Shield/ZCShieldActor.h"
#include "Physics/ZCCollision.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCShieldActor)

AZCShieldActor::AZCShieldActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void AZCShieldActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AZCShieldActor::Initialize(FZCActorTable* NewItem)
{
	Super::Initialize(NewItem);
	if (ItemInfo->ItemType != EItemType::Shield) return;

	ShieldInfo = static_cast<FZCShieldTable*>(Info);
}
