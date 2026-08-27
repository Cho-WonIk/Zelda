// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/PrimaryData/ZCPrimaryDataAsset.h"
#include "Physics/ZCShape.h"
#include "ZCActorPrimaryDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCActorPrimaryDataAsset : public UZCPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual EZCPrimaryAssetType GetZCPrimaryAssetType() const { return EZCPrimaryAssetType::Actor; }

public:
	UPROPERTY(EditAnywhere, Category = "Mesh", meta = (AllowedClasses = "StaticMesh,SkeletalMesh"))
	TSoftObjectPtr<UStreamableRenderAsset> Mesh;

	UPROPERTY(EditAnywhere, Category = "Collision", meta = (DisplayName = "상호작용 히트박스"))
	FZCShape Shape;
};
