// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Chaos/ChaosEngineInterface.h"

#include "Core/ZCGearUtility.h"
#include "Core/Module/ZCSimGearBaseModule.h"

#define ZC_API ZELDAPHYSICS_API

namespace ZCGear
{
	struct FZCSuspensionSettings
	{
		FZCSuspensionSettings()
		{}

		// 최대 좌우 회전 각도(도 단위)
		float MaxYawAngle;

		// 조향 보간 속도
		float SteerSpeed;

		// 초기 상대 회전값
		FQuat InitialRelativeRotation = FQuat::Identity;
	};

	class FZCSimGearSuspensionModule : public IZCSimGearBaseModule, public TZCSimGearModuleSettings<FZCSuspensionSettings>, public TZCGearModuleTypeable<FZCSimGearSuspensionModule>
	{
	public:
		DECLARE_GEAR_MODULE_TYPE(FZCSimGearSuspensionModule);

		ZC_API FZCSimGearSuspensionModule(const FZCSuspensionSettings& Settings);

		ZC_API virtual void Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs) override;

	private:
		float CurrentYawAngle = 0.0f;
	};
}

#undef ZC_API