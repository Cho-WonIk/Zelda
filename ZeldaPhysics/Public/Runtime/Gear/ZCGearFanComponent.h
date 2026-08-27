// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Gear/ZCGearBaseComponent.h"
#include "ZCGearFanComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class ZELDAPHYSICS_API UZCGearFanComponent : public UZCGearBaseComponent
{
	GENERATED_BODY()
	
public:
	virtual EZCGearType GetGearType() const override { return EZCGearType::Fan; }
	virtual ZCGear::IZCSimGearBaseModule* CreateNewGearModule() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear|Fan")
	float WindStrength = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear|Fan")
	float MaxWindSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear|Fan")
	float PGain = 5.0f;
};
