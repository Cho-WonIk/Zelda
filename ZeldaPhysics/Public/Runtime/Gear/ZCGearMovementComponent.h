// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "Core/ZCSimGearSimulation.h"
#include "Core/ZCSimGearEnum.h"
#include "ZCGearMovementComponent.generated.h"

namespace ZCGear
{
	class FZCSimGearGraph;
}

class FZCSimGearManager;

struct FZCSimGearManagerAsyncOutput;
struct FZCSimGearManagerAsyncInput;
struct FZCSimGearAsyncInput;
struct FZCSimGearAsyncOutput;

UCLASS()
class ZELDAPHYSICS_API UZCGearMovementComponent : public UMovementComponent
{
	GENERATED_BODY()
	
public:
	UZCGearMovementComponent(const FObjectInitializer& ObjectInitializer);

public:
	virtual bool ShouldCreatePhysicsState() const override { return true; }
	virtual void OnCreatePhysicsState() override;
	virtual void OnDestroyPhysicsState() override;

public:
	// 스스로 시뮬레이션 활성화(비동기이므로 버퍼에 기록되는 로직)
	void EnableSimulation();
	// 스스로 시뮬레이션 비활성화(비동기이므로 버퍼에 기록되는 로직)
	void DisableSimulation();

	// 물리 스레드에서 병렬로 실행되는 업데이트 함수
	void ParalleUpdate(float DeltaTime);

	// Update함수
	void Update(float DeltaTime);

	void FinalizeSimCallbackData(struct FZCSimGearManagerAsyncInput& Input);

	// 액터의 PostInitializeComponents에서 호출되어 FZCSimGearSimulation를 구성, OnCreatePhysicsState시점에는 하위 컴포넌트의 활성화가 안된 경우가 있음
	void CreateGearModules();

	// 게임 스레드에서 물리 스레드로 보낼 데이터 생성
	TUniquePtr<FZCSimGearAsyncInput> SetCurrentAsyncData(int32 InputIdx, FZCSimGearManagerAsyncOutput* CurrentOutput, FZCSimGearManagerAsyncOutput* NextOutput, float Alpha, int32 ManagerTimeStamp);

	void SetGearState(EZCGearState NewState);
	EZCGearState GetGearState() const { return GearState; }

protected:
	// FZCSimGearSimulation 생성
	void CreateGearSimulation();
	// FZCSimGearSimulation 소멸
	void DestroyGearSimulation();

private:
	// 재귀적으로 하위 컴포넌트들을 탐색하여 IZCGearBaseComponentInterface 구현체를 찾고 CreateNewGearModule을 통해 모듈을 생성 후 GearSimulationPT에 등록
	void CreateGearModulesRecursively(USceneComponent* ParentComponent, TArray<ZCGear::IZCSimGearBaseModule*>& OutModules);

	// 피직스 프록시 찾는 함수
	IPhysicsProxyBase* GetPhysicsProxy();

	// 피직스 프록시에서 물리 오브젝트 핸들을 찾는 함수
	Chaos::FPhysicsObjectHandle GetPhysicsObject(IPhysicsProxyBase* Proxy);

	// 게임 스레드에서 Solver 커맨드 큐에 작업 등록
	void EnqueueToPhysicsThread(TFunction<void()> Command);


public:
	// 물리 시뮬레이션에서 비동기 콜백 객체
	TUniquePtr<FZCSimGearSimulation> GearSimulationPT;
	
	// 해당 액터의 물리 스레드에서 게임 스레드로 전달할 데이터

	struct FZCSimGearAsyncInput* CurrentAsyncInput = nullptr;
	struct FZCSimGearAsyncOutput* CurrentAsyncOutput = nullptr;
	struct FZCSimGearAsyncOutput* NextAsyncOutput;
	float OutputInterAlpha;

protected:
	friend class FZCSimGearManager;
	friend class ZCGear::FZCSimGearGraph;

	int32 GearTreeNodeIndex = -1;

	EZCGearState GearState = EZCGearState::Disabled;

private:
	TArray<AActor*> ActorsToIgnore;

	// RootComponent의 프록시
	IPhysicsProxyBase* CachedPhysicsProxy = nullptr;
};
