// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameData/ZCItemTable.h"
#include "Struct/Struct/ZCItemStruct.h"
#include "ZCItemListObject.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCItemListObject : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType Type = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	Rarity Rarity = Rarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Durability = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InventoryIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsNewItem = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
	
	// 무기 = 공격력, 의상 = 방어력, 일반 = -1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Option = -1;

	// 무기정보에 나오는 무기 타입별 아이콘(양손검, 한손검, 창)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Info_Icon;

	void EmptyObject()
	{
		ID = -1;
		Type = EItemType::None;
		Rarity = Rarity::Common;
		Name = FString();
		Durability = -1;
		InventoryIndex = -1;
		bIsNewItem = true;
		Count = 0;
		Description = FString();
		Icon = nullptr;
		Option = -1;
	}

	void InitializeFromData(const FZCInventoryUIEntry& ItemKey, const uint8 ItemCount);
};
