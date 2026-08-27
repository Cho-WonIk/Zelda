// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZCPrimaryDataAsset.generated.h"

/**
 * 
 */

UENUM()
enum class EZCPrimaryAssetType : uint8
{
	None,
	Actor,
	Item,
	Weapon,
	Shield,
	Widget,
	Max
};

UCLASS()
class ZELDA_API UZCPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual EZCPrimaryAssetType GetZCPrimaryAssetType() const { return EZCPrimaryAssetType::None; }
};
