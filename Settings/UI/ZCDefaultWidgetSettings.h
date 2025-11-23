// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameData/PrimaryData/Widget/Wheel/ZCWheelSlotStylePrimaryDataAsset.h"
#include "ZCDefaultWidgetSettings.generated.h"

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "젤다 - UI 설정"))
class ZELDA_API UZCDefaultWidgetSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 기본 Wheel 비활성화시
	UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties), Category = "기본 휠 설정")
	TSoftObjectPtr<UZCWheelSlotStylePrimaryDataAsset> DefaultWheelSetting;

	// 기본 Wheel 활성화시
	UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties), Category = "기본 휠 설정(선택시)")
	TSoftObjectPtr<UZCWheelSlotStylePrimaryDataAsset> DefaultSelectWheelSetting;

	// 효과음
	UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties), Category = "휠 오픈 효과음")
	TSoftObjectPtr<USoundBase> OpenSound;
	UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties), Category = "휠 닫음 효과음")
	TSoftObjectPtr<USoundBase> CloseSound;

};