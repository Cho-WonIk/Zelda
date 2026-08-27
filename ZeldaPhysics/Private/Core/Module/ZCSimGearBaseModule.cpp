// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/Module/ZCSimGearBaseModule.h"

#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "PhysicsProxy/ClusterUnionPhysicsProxy.h"
#include "Chaos/ClusterUnionManager.h"

#include "Core/ZCSimGearSimulation.h"

namespace ZCGear
{
	void FZCSimGearOutputData::FillOutputState(const IZCSimGearBaseModule* Module)
	{
		if (!Module) return;

	}

	void FZCSimGearOutputData::Lerp(const FZCSimGearOutputData& InCurrent, const FZCSimGearOutputData& InNext, float Alpha)
	{
	}

	//---------------------------------------------------------------------------------

	void IZCSimGearBaseModule::SetActiveState(EZCGearState NewState)
	{
		ActiveState = NewState;

		// TODO : EZCSimGearPhysiscType::NoPhysics인 경우 오브젝트의 타입을 Kinematic으로 설정

	}

	void IZCSimGearBaseModule::SetPhysicsProxy(IPhysicsProxyBase* Proxy)
	{
		CachedPhysicsProxy = Proxy;
	}

	Chaos::FPBDRigidParticleHandle* IZCSimGearBaseModule::GetParticleHandle(IPhysicsProxyBase* Proxy) const
	{
		if (!Proxy) return nullptr;

		switch (Proxy->GetType())
		{
		case EPhysicsProxyType::ClusterUnionProxy:
		{
			if (Chaos::FClusterUnionPhysicsProxy* CUProxy = static_cast<Chaos::FClusterUnionPhysicsProxy*>(Proxy))
			{
				// Evolution 객체, 전체 물리 시뮬레이션의 변화시키는 역할을 담당하는 객체
				Chaos::FPBDRigidsEvolutionGBF& Evolution = *static_cast<Chaos::FPBDRigidsSolver*>(CUProxy->GetSolver<Chaos::FPBDRigidsSolver>())->GetEvolution();
				// 클러스터 유니언 객체를 관리하는 매니저
				Chaos::FClusterUnionManager& ClusterUnionManager = Evolution.GetRigidClustering().GetClusterUnionManager();
				// 현재 프록시와 연결된 클러스터 유니언의 인덱스를 얻음
				const Chaos::FClusterUnionIndex& CUIndex = CUProxy->GetClusterUnionIndex();
				// 해당 인덱스를 통해 클러스터 유니언 객체를 찾음
				if (Chaos::FClusterUnion* ClusterUnion = ClusterUnionManager.FindClusterUnion(CUIndex))
				{
					// 클러스터 유니언에서 내부 클러스터 핸들을 얻음
					if (Chaos::FPBDRigidClusteredParticleHandle* ClusterHandle = ClusterUnion->InternalCluster)
					{
						return ClusterHandle;
					}
				}
			}
		}
		break;

		case EPhysicsProxyType::SingleParticleProxy:
		{
			if (Chaos::FSingleParticlePhysicsProxy* ParticleProxy = static_cast<Chaos::FSingleParticlePhysicsProxy*>(Proxy))
			{
				auto* HandleLowLevel = ParticleProxy->GetHandle_LowLevel();
				if (HandleLowLevel)
				{
					if (Chaos::FPBDRigidParticleHandle* RigidHandle = HandleLowLevel->CastToRigidParticle())
					{
						return RigidHandle;
					}
				}
			}
		}
		break;

		default:
			break;
		}

		return nullptr;
	}

	void IZCSimGearBaseModule::AddLocalForceAtPosition(const FVector& Force, const FVector& Position, bool bAllowSubstepping /*= true*/, bool bIsLocalForce /*= false*/, bool bLevelSlope /*= false*/, const FColor& DebugColorIn /*= FColor::Blue*/)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().Add(FZCDeferredForcesGear::FZCApplyForceAtPositionData(
				CachedPhysicsProxy,
				RelativeTransform,
				Force,
				Position,
				bAllowSubstepping,
				bIsLocalForce,
				bLevelSlope,
				DebugColorIn
			));
		}
	}

	void IZCSimGearBaseModule::AddForceAtCOMPosition(const FVector& Force, const FVector& OffsetFromCOM /*= FVector::ZeroVector*/, bool bAllowSubstepping /*= true*/, bool bLevelSlope /*= false*/, const FColor& DebugColorIn /*= FColor::Blue*/)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().AddCOM(FZCDeferredForcesGear::FZCApplyForceAtPositionData(
				CachedPhysicsProxy,
				RelativeTransform,
				Force,
				OffsetFromCOM,
				bAllowSubstepping,
				false,
				bLevelSlope,
				DebugColorIn
			));
		}
	}

	void IZCSimGearBaseModule::AddLocalForce(const FVector& Force, bool bAllowSubstepping /*= true*/, bool bIsLocalForce /*= false*/, bool bLevelSlope /*= false*/, const FColor& DebugColorIn /*= FColor::Blue*/)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().Add(FZCDeferredForcesGear::FZCApplyForceData(
				CachedPhysicsProxy,
				RelativeTransform,
				Force,
				bAllowSubstepping,
				false,
				bIsLocalForce,
				bLevelSlope,
				DebugColorIn
			));
		}
	}

	void IZCSimGearBaseModule::AddLocalTorque(const FVector& Torque, bool bAllowSubstepping /*= true*/, bool bAccelChangeIn /*= true*/, const FColor& DebugColorIn /*= FColor::Magenta*/)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().Add(FZCDeferredForcesGear::FZCAddTorqueData(
				CachedPhysicsProxy,
				RelativeTransform,
				Torque,
				bAllowSubstepping,
				bAccelChangeIn,
				DebugColorIn
			));
		}
	}

	void IZCSimGearBaseModule::SetVelocity(const FVector& Velocity, bool bAddToCurrent /*= false*/, const FColor& DebugColorIn /*= FColor::Cyan*/)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().Add(FZCDeferredForcesGear::FZCSetVelocityData(
				CachedPhysicsProxy,
				RelativeTransform,
				Velocity,
				bAddToCurrent,
				DebugColorIn
			));
		}
	}

	void IZCSimGearBaseModule::SetAngularVelocity(const FVector& AngularVelocity, bool bAddToCurrent /*= false*/, const FColor& DebugColorIn /*= FColor::Cyan*/)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().Add(FZCDeferredForcesGear::FZCSetAngularVelocityData(
				CachedPhysicsProxy,
				RelativeTransform,
				AngularVelocity,
				bAddToCurrent,
				DebugColorIn
			));
		}
	}

	void IZCSimGearBaseModule::SetRotation(const FQuat& Rotation, bool bAddToCurrent /*= false*/)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().Add(FZCDeferredForcesGear::FZCSetRotationData(
				CachedPhysicsProxy,
				RelativeTransform,
				Rotation,
				bAddToCurrent
			));
		}
	}

	void IZCSimGearBaseModule::SetTorque(const FVector& Torque, bool bIsLocalTorque, const FColor& DebugColorIn)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().Add(FZCDeferredForcesGear::FZCSetTorqueData(
				CachedPhysicsProxy,
				RelativeTransform,
				Torque,
				bIsLocalTorque,
				DebugColorIn
			));
		}
	}

	void IZCSimGearBaseModule::AddSpeedLimitedForce(const FVector& Force, float MaxSpeed, bool bAllowSubstepping, bool bAccelChangeIn, bool bIsLocalForce, bool bLevelSlope, const FColor& DebugColorIn)
	{
		if (OwnerGearSimulation && CachedPhysicsProxy)
		{
			OwnerGearSimulation->AccessDeferredForces().Add(FZCDeferredForcesGear::FZCApplySpeedLimitedForceData(
				CachedPhysicsProxy,
				RelativeTransform,
				Force,
				MaxSpeed,
				bAllowSubstepping,
				bAccelChangeIn,
				bIsLocalForce,
				bLevelSlope,
				DebugColorIn
			));
		}
	}
}