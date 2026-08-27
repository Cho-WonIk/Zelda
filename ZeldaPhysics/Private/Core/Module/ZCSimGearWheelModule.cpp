// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Module/ZCSimGearWheelModule.h"

#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

namespace ZCGear
{
	FZCSimGearWheelModule::FZCSimGearWheelModule(const FZCWheelSettings& Settings)
		: TZCSimGearModuleSettings<FZCWheelSettings>(Settings)
	{
		//DisabledPhysicsType = EZCSimGearPhysiscType::CustomPhysics;
	}

	void FZCSimGearWheelModule::Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs)
	{
		float Speed = Setup().DefaultSpeed;

		if (Inputs.HasValidInputContainer())
		{
			const FZCSimGearWheelModuleInput& WheelInput = Inputs.ModuleInputContainer->WheelInput;

			// 피치 입력에 따른 바퀴 회전 속도 설정
			WheelInput.Pitch >= 0.0f ? Speed = Setup().DefaultSpeed : Speed = -Setup().DefaultSpeed;

			WheelInput.Yaw;

		}

		SetAngularVelocity(FVector(0.f, Speed, 0.f));
	}
}