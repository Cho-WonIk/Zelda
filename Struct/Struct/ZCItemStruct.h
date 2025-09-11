// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZCItemStruct.generated.h"

USTRUCT(BlueprintType)
struct FZCInventoryUIEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 ID = -1;

	UPROPERTY(EditAnywhere)
	EItemType Type = EItemType::None;

	UPROPERTY(EditAnywhere)
	int32 Durability = -1;

	UPROPERTY(EditAnywhere)
	int32 InventoryIndex = -1;

	bool operator==(const FZCInventoryUIEntry& Other) const
	{
		return Type == Other.Type && ID == Other.ID && Durability == Other.Durability && InventoryIndex == Other.InventoryIndex;
	}
};
