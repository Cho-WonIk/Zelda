// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicsPublic.h"
#include "Chaos/SimCallbackInput.h"
#include "Chaos/SimCallbackObject.h"

#include "Core/Module/ZCSimGearBaseModule.h"
#include "Core/ZCSimGearInput.h"

#define ZC_API ZELDAPHYSICS_API

class UZCGearMovementComponent;

/*=============================================================================================================================*/
// 각기어의 상태 입력 데이터를 게임 스레드에서 물리 스레드로 전달하기 위한 구조체
// 상태 데이터에는 사용할 콜리전 채널 정보, 인게임 정보(플레이어 입력, 외부 게임 이벤트 힘 등)을 포함
struct FZCSimGearPhysicsInputData
{
	FZCSimGearPhysicsInputData()
		: SharedModuleInputs(nullptr)
	{}
	virtual ~FZCSimGearPhysicsInputData()
	{
	}

	mutable ECollisionChannel CollisionChannel;
	mutable FCollisionQueryParams TraceParams;
	mutable FCollisionResponseContainer TraceCollisionResponses;

	// 물리 스레드 전역 입력 컨테이너 참조 (블랙보드)
	// OnPreSimulate_Internal에서 설정되며, 물리 스레드 동안 읽기 전용으로 사용
	mutable const ZCGear::FZCSimGearModuleInputContainer* SharedModuleInputs;

	void Reset()
	{
		SharedModuleInputs = nullptr;
	}
};

struct FZCSimGearAsyncInput
{
	FZCSimGearAsyncInput() : Gear(nullptr)
	{
		Proxy = nullptr;
	}

	virtual ~FZCSimGearAsyncInput() = default;

	// 기어 시뮬레이션 수행, 물리 스레드
	ZC_API virtual TUniquePtr<struct FZCSimGearAsyncOutput> Simulate(UWorld* World, const float DeltaSeconds, const float TotalSeconds) const;
	// 기어 시뮬레이션의 결과를 물리 스레드에서 게임 스레드로 적용, 물리 스레드에서 실행
	ZC_API virtual void ApplyDeferredForces() const;

	void SetGear(UZCGearMovementComponent* InGear) { Gear = InGear; }
	UZCGearMovementComponent* GetGear() const { return Gear; }

	// 입력 데이터 처리(물리 시뮬레이션 전에 호출)
	ZC_API virtual void ProcessInputs();

	// 플레이어 입력을 받는지 여부 확인
	bool IsAcceptingPlayerInput() const;

	IPhysicsProxyBase* Proxy;

	mutable FZCSimGearPhysicsInputData GearPhysicsInputData;

private:
	UZCGearMovementComponent* Gear;
};

struct FZCSimGearManagerAsyncInput : public Chaos::FSimCallbackInput
{
	TArray<TUniquePtr<FZCSimGearAsyncInput>> GearInputs;

	TWeakObjectPtr<UWorld> World;

	int32 TimeStamp = INDEX_NONE;

	// 물리 스레드 전역 입력 컨테이너 (블랙보드)
	// 게임 스레드에서 복사되어 물리 스레드 동안 읽기 전용으로 사용
	ZCGear::FZCSimGearModuleInputContainer SharedModuleInputContainer;

	// 플레이어 입력 처리 완료 여부
	uint8 bInputsProcessed : 1 = false;

	void Reset()
	{
		GearInputs.Reset();
		World.Reset();
		SharedModuleInputContainer.Reset();
		bInputsProcessed = false;
	}

	// 입력 버퍼에서 공유 모듈 입력 컨테이너로 추출
	ZC_API void PrepareSharedInputs(const ZCGear::FZCSimGearInputBufferContainer& InputBuffer);
};

/*=============================================================================================================================*/

// 각 기어의 상태 출력 데이터를 물리 스레드에서 게임 스레드로 전달하기 위한 구조체
// 하나의 액터에는 여러개의 IZCSimGearBaseModule로부터 받은 출력 데이터를 저장
struct FZCSimGearPhysicsOutput
{
	FZCSimGearPhysicsOutput() {}
	~FZCSimGearPhysicsOutput()
	{
		Clean();
	}

	void Clean()
	{
		for (ZCGear::FZCSimGearOutputData* Data : GearOutputData)
		{
			delete Data;
		}
		GearOutputData.Empty();
	}

	TArray<ZCGear::FZCSimGearOutputData*> GearOutputData;
};

// 기어의 비동기 출력 데이터
// 비동기 처리를 위해 FZCSimGearPhysicsOutput를 렙핑
struct FZCSimGearAsyncOutput
{
	FZCSimGearAsyncOutput() : bValid(false)
	{ }

	virtual ~FZCSimGearAsyncOutput()
	{
		GearPhysicsOutput.Clean();
	}

	bool bValid;

	// 기어의 액터 출력 데이터
	FZCSimGearPhysicsOutput GearPhysicsOutput;
};

// 비동기 멀티스레드 환경에서 각 기어의 출력 데이터를 담기 위한 구조체
struct FZCSimGearManagerAsyncOutput : public Chaos::FSimCallbackOutput
{
	// 각 기어의 비동기 출력 데이터
	TArray<TUniquePtr<FZCSimGearAsyncOutput>> GearOutputs;

	int32 Timestamp = INDEX_NONE;

	void Reset()
	{
		GearOutputs.Reset();
	}
};


class FZCSimGearAsyncCallback : public Chaos::TSimCallbackObject<FZCSimGearManagerAsyncInput, FZCSimGearManagerAsyncOutput
	, Chaos::ESimCallbackOptions::Presimulate | Chaos::ESimCallbackOptions::Rewind | Chaos::ESimCallbackOptions::ContactModification>
{
public:
	ZC_API virtual FName GetFNameForStatId() const override;
private:
	ZC_API virtual void ProcessInputs_Internal(int32 PhysicsStep) override;
	ZC_API virtual void OnPreSimulate_Internal() override;
	ZC_API virtual void OnContactModification_Internal(Chaos::FCollisionContactModifier& Modifications) override;
};

#undef ZC_API