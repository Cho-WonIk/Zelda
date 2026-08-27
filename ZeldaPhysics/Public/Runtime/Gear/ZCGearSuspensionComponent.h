// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Gear/ZCGearBaseComponent.h"
#include "ZCGearSuspensionComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class ZELDAPHYSICS_API UZCGearSuspensionComponent : public UZCGearBaseComponent
{
	GENERATED_BODY()
	
public:
	virtual EZCGearType GetGearType() const override { return EZCGearType::Suspension; }
	virtual ZCGear::IZCSimGearBaseModule* CreateNewGearModule() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear|Suspension")
	float MaxYawAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear|Suspension")
	float SteerSpeed = 5.0f;
};
