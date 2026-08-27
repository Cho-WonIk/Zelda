// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameData/PrimaryData/Widget/Wheel/ZCWheelSlotStylePrimaryDataAsset.h"
#include "ZCDefaultWidgetSettings.generated.h"

class UZCHUDWidget;

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "젤다 - UI 설정"))
class ZELDA_API UZCDefaultWidgetSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// --- Wheel 카테고리 ---

	// 기본 Wheel 비활성화시
	UPROPERTY(EditAnywhere, config, Category = "Wheel", meta = (ShowOnlyInnerProperties, DisplayName = "기본 휠 설정"))
	TSoftObjectPtr<UZCWheelSlotStylePrimaryDataAsset> DefaultWheelSetting;

	// 기본 Wheel 활성화시
	UPROPERTY(EditAnywhere, config, Category = "Wheel", meta = (ShowOnlyInnerProperties, DisplayName = "기본 휠 설정(선택시)"))
	TSoftObjectPtr<UZCWheelSlotStylePrimaryDataAsset> DefaultSelectWheelSetting;

	// 효과음
	UPROPERTY(EditAnywhere, config, Category = "Wheel", meta = (ShowOnlyInnerProperties, DisplayName = "휠 오픈 효과음"))
	TSoftObjectPtr<USoundBase> OpenSound;

	UPROPERTY(EditAnywhere, config, Category = "Wheel", meta = (ShowOnlyInnerProperties, DisplayName = "휠 닫음 효과음"))
	TSoftObjectPtr<USoundBase> CloseSound;

public:
	// --- HUD 카테고리 ---

	UPROPERTY(EditAnywhere, config, Category = "HUD", meta = (DisplayName = "인게임 HUD 클래스"))
	TSubclassOf<class UZCHUDWidget> ZCInGameHUDClass;
};