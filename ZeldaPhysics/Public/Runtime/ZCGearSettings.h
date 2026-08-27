// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Runtime/Input/ZCGearInputModifier.h"
#include "ZCGearSettings.generated.h"

UCLASS(Config = Engine, DefaultConfig, meta = (DisplayName = "젤다 - 기어 설정"))
class ZELDAPHYSICS_API UZCGearSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "Modifier", meta = (DisplayName = "기어 입력 모디파이어 목록"))
	TArray<TSoftObjectPtr<UZCGearInputModifier>> WheelGearInputModifiers;

};
