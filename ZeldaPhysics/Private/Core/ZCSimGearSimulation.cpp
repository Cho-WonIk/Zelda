// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ZCSimGearSimulation.h"

#include "PhysicsProxy/ClusterUnionPhysicsProxy.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "Chaos/ClusterUnionManager.h"

#include "Core/ZCSimGearAsyncCallback.h"
#include "Core/Module/ZCSimGearBaseModule.h"
#include "Core/ZCSimGearInput.h"

FZCSimGearSimulation::FZCSimGearSimulation()
{
}

FZCSimGearSimulation::~FZCSimGearSimulation()
{
	Terminate();
}

void FZCSimGearSimulation::Initialize(TArray<ZCGear::IZCSimGearBaseModule*>&& InSimGearModules)
{
	SimGearModules = MoveTemp(InSimGearModules);

}

void FZCSimGearSimulation::Terminate()
{
	for (auto* Module : SimGearModules)
	{
		if (Module)
		{
			Module->OnTermination_External();
			delete Module;
		}
	}
	SimGearModules.Empty();
}

void FZCSimGearSimulation::CacheRootParticle(IPhysicsProxyBase* Proxy)
{	
	RootParticleHandle = nullptr;

	if (!Proxy) return;

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
						RootParticleHandle = ClusterHandle;
					}
				}
			}
		}
		break;

		case EPhysicsProxyType::SingleParticleProxy:
		{
			if (Chaos::FSingleParticlePhysicsProxy* ParticleProxy = static_cast<Chaos::FSingleParticlePhysicsProxy*>(Proxy))
			{
				RootParticleHandle = ParticleProxy->GetHandle_LowLevel() ? ParticleProxy->GetHandle_LowLevel()->CastToRigidParticle() : nullptr;
			}
		}
		break;

		default:
			break;
	}

	//UE_LOG(LogTemp, Warning, TEXT("FZCSimGearSimulation::CacheRootParticle - Cached root particle handle: %p"), RootParticleHandle);
}

void FZCSimGearSimulation::Simulate(float DeltaSeconds, const FZCSimGearAsyncInput& InputData, const FZCSimGearAsyncOutput& OutputData)
{
	Chaos::EnsureIsInPhysicsThreadContext();

	if (SimGearModules.Num() <= 0) return;

	// 모듈에 전달할 입력 데이터 준비
	ZCGear::FZCSimGearAllInputs AllInputs;
	AllInputs.ModuleInputContainer = InputData.GearPhysicsInputData.SharedModuleInputs;

	// 모듈에 대해 시뮬레이션 수행
	for (ZCGear::IZCSimGearBaseModule* Module : SimGearModules)
	{
		if (Module && Module->GetCurrentPhysicsType() == EZCSimGearPhysiscType::CustomPhysics)
		{
			Module->Simulate(DeltaSeconds, AllInputs);
		}
	}
}

void FZCSimGearSimulation::ApplyDeferredForces(IPhysicsProxyBase* Proxy)
{
	Chaos::EnsureIsInPhysicsThreadContext();
	
	if (Proxy)
	{
		DeferredForces.Apply();
	}
}

void FZCSimGearSimulation::UpdateGearState()
{
	bool bShouldSimulate = false;

	for (ZCGear::IZCSimGearBaseModule* Module : SimGearModules)
	{
		if (Module)
		{
			Module->SetActiveState(CurrentGearState);
			if (Module->GetCurrentPhysicsType() == EZCSimGearPhysiscType::CustomPhysics)
			{
				bShouldSimulate = true;
			}
		}
	}

	if (bShouldSimulate)
	{
		UE_LOG(LogTemp, Verbose, TEXT("FZCSimGearSimulation::UpdateGearState - Simulation Enabled"));
	}

	bIsSimulating = bShouldSimulate;
}
