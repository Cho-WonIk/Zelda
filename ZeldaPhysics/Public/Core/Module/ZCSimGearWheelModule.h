// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Core/ZCGearUtility.h"
#include "Core/Module/ZCSimGearBaseModule.h"

#define ZC_API ZELDAPHYSICS_API

namespace ZCGear
{
	struct FZCWheelSettings
	{
		FZCWheelSettings()
			: DefaultSpeed(10.0f)
			, MaxYawAngle(22.5f)
		{}

		float DefaultSpeed;

		// 최대 좌우 회전 각도( 도 단우)
		float MaxYawAngle;

		// 차체(Source)와 바퀴(Wheel) 간의 초기 상대 회전값
		FQuat InitialRelativeRotation = FQuat::Identity;

		FVector InitialRelativeLocation = FVector::ZeroVector;

		// 부모의 물리 프록시
		IPhysicsProxyBase* SourcePhysicsProxy = nullptr;
	};

	class FZCSimGearWheelModule : public IZCSimGearBaseModule, public TZCSimGearModuleSettings<FZCWheelSettings>, public TZCGearModuleTypeable<FZCSimGearWheelModule>
	{
	public:
		DECLARE_GEAR_MODULE_TYPE(FZCSimGearWheelModule);

		ZC_API FZCSimGearWheelModule(const FZCWheelSettings& Settings);

		ZC_API virtual void Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs) override;

	private:
		// 조향 각도 (X축 회전)
		float CurrentYawAngle = 0.0f;

		// 굴림 각도 누적값 (Y축 회전)
		float CurrentRollAngle = 0.0f;
	};
}

#undef ZC_API