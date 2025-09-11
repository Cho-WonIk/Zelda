// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/PrimaryData/ZCPrimaryDataAsset.h"
#include "ZCItemPrimaryData.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCItemPrimaryData : public UZCPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TSoftObjectPtr<UStaticMesh> Mesh;

};
