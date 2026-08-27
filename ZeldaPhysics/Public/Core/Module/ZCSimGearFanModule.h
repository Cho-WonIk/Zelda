// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Core/ZCGearUtility.h"
#include "Core/Module/ZCSimGearBaseModule.h"

#define ZC_API ZELDAPHYSICS_API

namespace ZCGear
{
	struct FZCFanSettings
	{
		FZCFanSettings()
			: DefaultForce(1000.0f)
			, Axis(EZCSimGearAxis::X)
			, MaxSpeed(10.0f)
			, PGain(5.0f)
			, IGain(0.1f)
			, DGain(0.5f)
		{}

		float DefaultForce = 1000.0f;
		float MaxSpeed = 10.0f;
		float PGain = 5.0f;
		float IGain;
		float DGain;
		EZCSimGearAxis Axis = EZCSimGearAxis::X;
	};

	class FZCSimGearFanModule : public IZCSimGearBaseModule, public TZCSimGearModuleSettings<FZCFanSettings>, public TZCGearModuleTypeable<FZCSimGearFanModule>
	{
	public:
		DECLARE_GEAR_MODULE_TYPE(FZCSimGearFanModule);

		ZC_API FZCSimGearFanModule(const FZCFanSettings& Settings);

		ZC_API virtual void Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs) override;

	private:
		float ErrorSum = 0.0f;     // 오차 누적 (Integral)
		float PreviousError = 0.0f; // 이전 오차 (Derivative용)
	};
}

#undef ZC_API
