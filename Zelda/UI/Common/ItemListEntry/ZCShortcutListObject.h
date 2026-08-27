// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameData/Table/ZCItemTable.h"
#include "ZCShortcutListObject.generated.h"

namespace CustomFont
{
	namespace Weapon
	{
		// 선언만
		extern const FString OneHand;
		extern const FString TwoHand;
		extern const FString Spear;
	}
}

UCLASS()
class ZELDA_API UZCShortcutListObject : public UObject
{
	GENERATED_BODY()

public:
	UZCShortcutListObject();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType Type = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	Rarity Rarity = Rarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name = FText();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description = FText();

	// 무기 = 공격력, 방패/방어구 = 방어력, 일반 = -1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Option = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InventoryIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsNewItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// 무기정보에 나오는 무기 타입별 아이콘(양손검, 한손검, 창)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Info = FString();
};
