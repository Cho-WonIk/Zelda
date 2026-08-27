// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Core/ZCGearUtility.h"
#include "Core/Module/ZCSimGearBaseModule.h"

#define ZC_API ZELDAPHYSICS_API

namespace ZCGear
{
	struct FZCInputSettings
	{
		
	};

	class FZCSimGearInputModule : public IZCSimGearBaseModule, public TZCSimGearModuleSettings<FZCInputSettings>, public TZCGearModuleTypeable<FZCSimGearInputModule>
	{
	public:
		DECLARE_GEAR_MODULE_TYPE(FZCSimGearInputModule);

		ZC_API FZCSimGearInputModule(const FZCInputSettings& Settings);

		ZC_API virtual void Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs) override;
	};
}

#undef ZC_API