// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Core/ZCGearUtility.h"
#include "Core/Module/ZCSimGearBaseModule.h"

#define ZC_API ZELDAPHYSICS_API

namespace ZCGear
{
	struct FZCBalloonSettings
	{
		FZCBalloonSettings()
			: MaxForce(500.0f)
			, MaxSpeed(5.0f)
		{}
		float MaxForce = 500.0f;
		float MaxSpeed = 5.0f;
	};
	class FZCSimGearBalloonModule : public IZCSimGearBaseModule, public TZCSimGearModuleSettings<FZCBalloonSettings>, public TZCGearModuleTypeable<FZCSimGearBalloonModule>
	{
	public:
		DECLARE_GEAR_MODULE_TYPE(FZCSimGearBalloonModule);

		ZC_API FZCSimGearBalloonModule(const FZCBalloonSettings& Settings);

		ZC_API virtual void Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs) override;

	private:
	};

}

#undef ZC_API
