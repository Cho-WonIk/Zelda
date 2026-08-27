// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Gear/ZCGearBaseComponent.h"
#include "ZCGearWheelComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class ZELDAPHYSICS_API UZCGearWheelComponent : public UZCGearBaseComponent
{
	GENERATED_BODY()
	
public:
	virtual EZCGearType GetGearType() const override { return EZCGearType::Wheel; }
	virtual ZCGear::IZCSimGearBaseModule* CreateNewGearModule() const override;
};
