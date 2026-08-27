// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Slate/WidgetTransform.h"
#include "GameData/Enum/ZCWidget.h"
#include "GameData/PrimaryData/Widget/Wheel/ZCWheelSlotStylePrimaryDataAsset.h"
#include "ZCWheelStruct.generated.h"


USTRUCT(BlueprintType)
struct FWheelItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "이름"))
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "표시 이름"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "아이콘"))
	TObjectPtr<UObject> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "아이콘 크기"))
	FVector2D IconSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "아이콘 트랜스폼"))
	FWidgetTransform IconTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "정보 텍스트"))
	FText InfoText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "정보 텍스트 트랜스폼"))
	FWidgetTransform InfoTextTransform;
};

USTRUCT(BlueprintType)
struct FWheelSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "슬롯 스타일"))
	TObjectPtr<UZCWheelSlotStylePrimaryDataAsset> SlotStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "슬롯 스타일(활성화)"))
	TObjectPtr<UZCWheelSlotStylePrimaryDataAsset> SlotActiveStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "스케일"))
	float Scaling = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "활성화 상태"))
	uint8 bIsSelected : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "아이템 유무"))
	uint8 bHasItem : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "아이템"))
	FWheelItem Item;
};
