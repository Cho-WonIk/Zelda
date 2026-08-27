// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/ScopeRWLock.h"
#include "Misc/TransactionallySafeRWLock.h"

#include "Core/ZCSimGearEnum.h"
#include "Core/ZCDeferredForcesGear.h"
#include "Runtime/ZCGearType.h"

#define ZC_API ZELDAPHYSICS_API

namespace ZCGear
{
	class IZCSimGearBaseModule;
}

struct FZCSimGearAsyncInput;
struct FZCSimGearAsyncOutput;

/**모든 액터에 적용되어 최소 단위의 물리 시뮬레이션을 담당
 * 해당 물리 시뮬레이션의 호출 순서는 상위 클래스에서 구현
 * 해당 클래스의 시뮬레이션은 배열의 순서대로 각 모듈의 Simulate 함수 호출
 * 하나의 액터에서 소유중이 시뮬레이션 모듈의 계층 구조를 DFS에 따라 순회한 결과를 배열의 형태로 저장(하위 모듈의 상태는 상위 모듈의 시뮬레이션 결과에 영향을 받을 수 있음)
 */
class FZCSimGearSimulation
{

public:
	FZCSimGearSimulation();
	virtual ~FZCSimGearSimulation();

public:
	ZC_API virtual void Initialize(TArray<ZCGear::IZCSimGearBaseModule*>&& InSimGearModules);
	ZC_API virtual void Terminate();

	ZC_API void CacheRootParticle(IPhysicsProxyBase* Proxy);

	ZC_API virtual void Simulate(float DeltaSeconds, const FZCSimGearAsyncInput& InputData, const FZCSimGearAsyncOutput& OutputData);

	ZC_API void ApplyDeferredForces(IPhysicsProxyBase* Proxy);

	FZCDeferredForcesGear& AccessDeferredForces() { return DeferredForces; }
	const FZCDeferredForcesGear& GetDeferredForces() const { return DeferredForces; }

	EZCGearState GetCurrentGearState() const { return CurrentGearState; }

	void SetCurrentGearState(EZCGearState NewState) { CurrentGearState = NewState; UpdateGearState(); }

	bool IsSimulating() const { return bIsSimulating; }

	void SetAcceptingPlayerInput(bool bAcceptInput) { bAcceptsPlayerInput = bAcceptInput; }
	bool IsAcceptingPlayerInput() const { return bAcceptsPlayerInput; }

	Chaos::FPBDRigidParticleHandle* GetRootParticleHandle() const { return RootParticleHandle; }

protected:
	void UpdateGearState();

protected:
	TArray<ZCGear::IZCSimGearBaseModule*> SimGearModules;

	Chaos::FPBDRigidParticleHandle* RootParticleHandle = nullptr;

	FZCDeferredForcesGear DeferredForces;

	EZCGearState CurrentGearState = EZCGearState::Enabled;

	// 커스텀 시뮬레이션이 필요한지 여부
	uint8 bIsSimulating : 1 = true;

	// 플레이어의 입력을 받는지 여부
	uint8 bAcceptsPlayerInput : 1 = false;
};

#undef ZC_API