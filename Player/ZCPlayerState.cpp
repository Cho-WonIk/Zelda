// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ZCPlayerState.h"
#include "Component/Inventory/ZCInventoryComponent.h"

AZCPlayerState::AZCPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UZCInventoryComponent>(TEXT("InventoryComponent"));
}
