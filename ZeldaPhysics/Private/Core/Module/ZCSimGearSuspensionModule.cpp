// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Module/ZCSimGearSuspensionModule.h"
#include "Chaos/PBDSuspensionConstraints.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "PhysicsProxy/SuspensionConstraintProxy.h"
#include "PBDRigidsSolver.h"

#include "Core/ZCSimGearSimulation.h"

namespace ZCGear
{
	FZCSimGearSuspensionModule::FZCSimGearSuspensionModule(const FZCSuspensionSettings& Settings)
		: TZCSimGearModuleSettings<FZCSuspensionSettings>(Settings)
		, CurrentYawAngle(0.0f)
	{
		//DisabledPhysicsType = EZCSimGearPhysiscType::CustomPhysics;
	}

	void FZCSimGearSuspensionModule::Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs)
	{
		if (Inputs.HasValidInputContainer())
		{
			const FZCSimGearWheelModuleInput& WheelInput = Inputs.ModuleInputContainer->WheelInput;

			// 요 입력에 따른 좌우 축 회전 설정

			// 목표 Yaw 각도 계산
			const float MaxYaw = Setup().MaxYawAngle;
			const float TargetYawAngle = FMath::Clamp(WheelInput.Yaw * MaxYaw, -MaxYaw, MaxYaw);

			// 현재 각도를 목표 각도로 보간
			CurrentYawAngle = FMath::FInterpTo(CurrentYawAngle, TargetYawAngle, DeltaTime, Setup().SteerSpeed);

			const FQuat InitialRotation = Setup().InitialRelativeRotation;
			// 루트 파티클의 월드 회전을 가져옴
			FQuat BodyRotation = FQuat::Identity;
			if (OwnerGearSimulation)
			{
				if (Chaos::FPBDRigidParticleHandle* RootHandle = OwnerGearSimulation->GetRootParticleHandle())
				{
					BodyRotation = RootHandle->GetR();
				}
			}

			// CockpitDirection을 포함하는 평면 위에서 조향 회전 적용
			FVector CockpitDir = Inputs.ModuleInputContainer->CockpitDirection;

			FVector WorldSteerAxis = FVector::UpVector;
			if (CockpitDir.SizeSquared() > KINDA_SMALL_NUMBER)
			{
				CockpitDir.Normalize();
				WorldSteerAxis = FVector::UpVector - FVector::DotProduct(FVector::UpVector, CockpitDir) * CockpitDir;

				if (WorldSteerAxis.SizeSquared() > KINDA_SMALL_NUMBER)
				{
					WorldSteerAxis.Normalize();
				}
				else
				{
					WorldSteerAxis = FVector::ForwardVector;
				}
			}

			// 월드 공간에서 조향 회전 적용
			const FQuat WorldYawOffset = FQuat(WorldSteerAxis, FMath::DegreesToRadians(CurrentYawAngle));

			// 베이스 회전(바디 + 초기 상대 회전) 위에 월드 공간 조향을 적용
			const FQuat BaseRotation = BodyRotation * InitialRotation;
			const FQuat DesiredRotation = WorldYawOffset * BaseRotation;

			SetRotation(DesiredRotation);

		}
	}
}