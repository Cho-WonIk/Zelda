// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Gear/ZCGearBaseComponent.h"
#include "ZCGearBalloonComponent.generated.h"

/**
 * 
 */
UCLASS()
class ZELDAPHYSICS_API UZCGearBalloonComponent : public UZCGearBaseComponent
{
	GENERATED_BODY()
	
public:
	virtual EZCGearType GetGearType() const override { return EZCGearType::Balloon; }
	virtual ZCGear::IZCSimGearBaseModule* CreateNewGearModule() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear|Balloon")
	float MaxForce = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear|Balloon")
	float MaxSpeed = 5.0f;

};
