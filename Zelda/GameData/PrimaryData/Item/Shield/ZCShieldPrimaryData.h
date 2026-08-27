// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/PrimaryData/Item/ZCItemPrimaryData.h"
#include "ZCShieldPrimaryData.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCShieldPrimaryData : public UZCItemPrimaryData
{
	GENERATED_BODY()
	
public:
	virtual EZCPrimaryAssetType GetZCPrimaryAssetType() const { return EZCPrimaryAssetType::Shield; }
};
