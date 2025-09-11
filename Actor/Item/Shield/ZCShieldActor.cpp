// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Item/Shield/ZCShieldActor.h"
#include "Physics/ZCCollision.h"

AZCShieldActor::AZCShieldActor()
{
}

void AZCShieldActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AZCShieldActor::Initialize(FZCItemTable* NewItem)
{
	Super::Initialize(NewItem);
	if (Info->Type == EItemType::Shield)
	{
		ShieldInfo = static_cast<FZCShieldTable*>(Info);
	}
}
