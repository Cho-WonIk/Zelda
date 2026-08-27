// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/PrimaryData/ZCActorPrimaryDataAsset.h"
#include "ZCItemPrimaryData.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCItemPrimaryData : public UZCActorPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual EZCPrimaryAssetType GetZCPrimaryAssetType() const { return EZCPrimaryAssetType::Item; }
};
