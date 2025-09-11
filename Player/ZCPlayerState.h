// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameData/ZCItemTable.h"
#include "ZCPlayerState.generated.h"

 class UZCInventoryComponent;

UCLASS()
class ZELDA_API AZCPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AZCPlayerState();

	FORCEINLINE UZCInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<class UZCInventoryComponent> InventoryComponent;
};
