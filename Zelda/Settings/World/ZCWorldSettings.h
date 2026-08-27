// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NiagaraSystem.h"
#include "NativeGameplayTags.h"
#include "ZCWorldSettings.generated.h"

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "젤다 - 월드 설정"))
class ZELDA_API UZCWorldSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties, Categories = "Material"))
	TSoftObjectPtr< UMaterialInterface> UnacquiredItemOverlay;

	UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties, Categories = "Material"))
	TSoftObjectPtr< UMaterialInterface> UltraHandSelectable;

	/** UltraHand 머티리얼의 색상 파라미터 이름 */
	UPROPERTY(EditAnywhere, config, Category = "Material|Ultra Hand")
	FName UltraHandColorParameterName = FName(TEXT("Color"));

	/** UltraHand 기본 상태일 때의 색상 */
	UPROPERTY(EditAnywhere, config, Category = "Material|Ultra Hand")
	FLinearColor UltraHandDefaultColor;

	/** UltraHand로 선택되었을 때의 색상 */
	UPROPERTY(EditAnywhere, config, Category = "Material|Ultra Hand")
	FLinearColor UltraHandSelectedColor;

	/** 화학엔진 Tick 간격*/
	UPROPERTY(EditAnywhere, Config, Category = "Chemistry")
	float SpreadInterval = 0.2f;
};