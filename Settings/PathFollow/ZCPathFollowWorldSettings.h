// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Component/PathFollowing/Utils/ZCPathFollowUtils.h"
#include "Component/PathFollowing/ZCPathFollowingComponent.h"
#include "ZCPathFollowWorldSettings.generated.h"

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "젤다 - 경로 보정 설정"))
class ZELDA_API UZCPathFollowWorldSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** 경로 단순화 설정 */
    UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties), Category = "경로 단순화")
    FZCPathSimplificationSettings SimplificationSettings;

    /** 경로 부드럽게 처리 설정 */
    UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties), Category = "경로 부드럽게 처리")
    FZCPathSmoothingSettings SmoothingSettings;

    /** 내브메시 유효성 검사 설정 */
    UPROPERTY(EditAnywhere, config, meta = (ShowOnlyInnerProperties), Category = "NavMesh 유효성 검사")
    FZCNavMeshValidationSettings NavMeshValidationSettings;

    /** 부드러운 경로의 디버그 색상 */
    UPROPERTY(EditAnywhere, config, meta = (DisplayName = "부드러운 경로 색상", Category = "디버그 시각화"))
    FColor SmoothedPathDebugColor = FColor::Green;

    /** 원본 경로의 디버그 색상 */
    UPROPERTY(EditAnywhere, config, meta = (DisplayName = "원본 경로 색상", Category = "디버그 시각화"))
    FColor DefaultPathDebugColor = FColor::Red;
};
