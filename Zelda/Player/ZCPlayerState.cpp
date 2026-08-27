// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ZCPlayerState.h"
#include "Component/Inventory/ZCInventoryComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCPlayerState)

AZCPlayerState::AZCPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UZCInventoryComponent>(TEXT("InventoryComponent"));
}
