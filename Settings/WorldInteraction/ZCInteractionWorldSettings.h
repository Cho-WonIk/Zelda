// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NiagaraSystem.h"
#include "NativeGameplayTags.h"
#include "ZCInteractionWorldSettings.generated.h"

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "젤다 - 화학 엔진 설정"))
class ZELDA_API UZCInteractionWorldSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties, Categories = "Chemistry"), Category = "엘리먼트 맵")
	TMap<FGameplayTag, TSoftObjectPtr<UNiagaraSystem>> ElementReactionMap;
};