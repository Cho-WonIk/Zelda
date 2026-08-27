#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SceneManagement.h"
#include "Math/Color.h"
#include "PrimitiveSceneProxy.h"
#include "ZCActorPrimaryDataAssetEditorSettings.generated.h"

UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "ZC Actor Primary Data Asset Editor"))
class ZELDAEDITOR_API UZCActorPrimaryDataAssetEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Project Settings에서 보여질 카테고리/섹션 이름
	virtual FName GetCategoryName() const override { return TEXT("Editor"); }
	virtual FName GetSectionName() const override { return TEXT("ZC Mesh Debug Draw"); }

	/** 디버그 Shape 색상 */
	UPROPERTY(EditAnywhere, config, Category = "Shape Debug")
	FColor ShapeColor = FColor::Green;

	/** 라인/와이어 두께 */
	UPROPERTY(EditAnywhere, config, Category = "Shape Debug", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "10.0"))
	float ShapeThickness = 2.0f;

	/** Line Shape인 경우 끝점 구체 표시 여부 */
	UPROPERTY(EditAnywhere, config, Category = "Shape Debug | Line")
	uint8 bShowEndpointSpheres : 1 = false;

	/** bShowEndpointSpheres가 true일 때 끝점 구체 크기 */
	UPROPERTY(EditAnywhere, config, Category = "Shape Debug | Line", meta = (EditCondition = "bShowEndpointSpheres", EditConditionHides))
	float LineEndpointSphereSize = 5.0f;

	/** Depth Priority (Foreground 권장) */
	UPROPERTY(EditAnywhere, config, Category = "Shape Debug")
	TEnumAsByte<ESceneDepthPriorityGroup> DepthPriority = SDPG_Foreground;
};