// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/StrongObjectPtr.h"
#include "Core/ZCSimGearAsyncCallback.h"
#include "Runtime/ZCGearType.h"
#include "Runtime/Input/ZCGearInputModifier.h"
#include "Core/ZCSimGearInput.h"
#define ZC_API ZELDAPHYSICS_API

class FPhysScene_Chaos;

class FZCSimGearAsyncCallback;

namespace ZCGear
{
	class FZCSimGearGraph;
	class FZCSimGearGraphBuffer;
}

class UZCGearMovementComponent;

class FZCSimGearManager final
{
public:
	// 기어들의 물리 상태를 재생성할 때 업데이트 됨
	// 게임 중 값이 조정될 때 사용
	ZC_API FZCSimGearManager(FPhysScene* PhysScene);
	ZC_API ~FZCSimGearManager();

	// 월드 초기화 후 호출
	static ZC_API void OnPostWorldInitialization(UWorld* InWorld, const UWorld::InitializationValues);
	// 월드 종료 시 호출
	static ZC_API void OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources);
	// 화면에 디버그 정보를 표시
	static ZC_API void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

	FPhysScene_Chaos& GetScene() const { return Scene; }

	/** 물리 씬(FPhysScene)을 통해 해당 씬을 관리하는 기어 매니저 찾음 */
	static ZC_API FZCSimGearManager* GetManagerFromScene(FPhysScene* PhysScene);

public:
	// Gear등록
	ZC_API void AddGearSimulation(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation);
	// Gear제거
	ZC_API void RemoveGearSimulation(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation);

	// Gear 활성화 비활성화
	ZC_API void SetGearSimulationEnabled(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation, bool bEnabled);

	// 플레이어의 입력을 받는 Gear 등록
	ZC_API void RegisterPlayerInputGearSimulation(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation);
	// 플레이어의 입력을 받는 Gear 등록 해제
	ZC_API void UnRegisterPlayerInputGearSimulation(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation);

	// 플레이어 입력 버퍼에 기록
	ZC_API void PlayerInput(const FVector2D& InMoveInput);

	// 무게 중심 오프셋 설정 (비동기 전달용)
	ZC_API void SetCenterOfMassOffset(const FVector& InCenterOfMassOffset, const float& InMass);

	// 플레이어 캐릭터가 현재 바라보는 방향 설정 (비동기 전달용)
	ZC_API void SetPlayerViewVector(const FVector& InViewVector);

	// 입력 컨테이너에 대한 읽기 전용 접근
	const ZCGear::FZCSimGearInputBufferContainer& GetGearInputContainer() const { return *SimGearInputContainer.Get(); }

protected:
	// 기어 그래프 및 기어 그래프 버퍼
	TUniquePtr<ZCGear::FZCSimGearGraph> SimGearGraph;
	TUniquePtr<ZCGear::FZCSimGearGraphBuffer> SimGearGraphBuffer;

	// 기어 입력 버퍼 컨테이너
	TUniquePtr<ZCGear::FZCSimGearInputBufferContainer> SimGearInputContainer;

protected:
	ZC_API void RegisterCallbacks(UWorld* InWorld);
	ZC_API void UnregisterCallbacks();

	ZC_API void InitializeGearInputModifiers();
	ZC_API void DeinitializeGearInputModifiers();

	// 물리 스레드가 시뮬레이션을 시작하기 전에 호출되는 콜백보다 한발 앞서 호출, Update함수에서 실행, 게임 스레드
	ZC_API void ScenePreTick(FPhysScene* PhysScene, float DeltaTime);
	// 물리 스레드가 시뮬레이션을 시작하기 전 호출되는 콜백, 게임 스레드에서 실행
	ZC_API void Update(FPhysScene* PhysScene, float DeltaTime);
	// 물리 시뮬레이션 완료 후(PostTick) 호출되는 콜백
	ZC_API void PostUpdate(FChaosScene* PhysScene);

	// 플레이어의 입력 처리, 기어 입력 버퍼 컨테이너에서 모듈별 입력 추출 후 비동기 입력에 전달
	ZC_API void ProcessPlayerInput(float DeltaTime);

	// 입력 컨테이너에서 각 기어의 모듈 입력 데이터 추출 및 전달
	ZC_API void PrepareModuleInputs(FZCSimGearManagerAsyncInput* AsyncInput);

private:
	// 시뮬레이션할 기어 데이터를 병렬로 업데이트
	void ParallelUpdateGear(float DeltaTime);

private:
	static ZC_API TMap<FPhysScene*, FZCSimGearManager*> SceneToGearManagerMap;

	FPhysScene_Chaos& Scene;

	static ZC_API bool GInitialized;

	FDelegateHandle OnNetDriverCreatedHandle;
	FDelegateHandle OnPhysScenePreTickHandle;
	FDelegateHandle OnPhysScenePostTickHandle;

	static ZC_API FDelegateHandle OnPostWorldInitializationHandle;
	static ZC_API FDelegateHandle OnWorldCleanupHandle;

	// 물리 시뮬레이션과 게임	스레드 간의 비동기 콜백 객체
	class FZCSimGearAsyncCallback* AsyncCallback = nullptr;

	int32 TimeStamp;
	int32 SubStepCount;

	// 비동기 입력/출력 상태
	TArray<Chaos::TSimCallbackOutputHandle<FZCSimGearManagerAsyncOutput>> PendingOutputs;
	Chaos::TSimCallbackOutputHandle<FZCSimGearManagerAsyncOutput> LatestOutput;

private:
	// 기어 입력 변조 에셋 맵
	TMap<EZCGearType, TStrongObjectPtr<class UZCGearInputModifier>> GearInputModifierMap;

	// --- 디버깅용 변수 ---
	//TWeakObjectPtr<UZCGearMovementComponent> LastAddedGearForDebug;
	//float DebugTimer = 0.0f;
	//bool bDebugCurrentState = false;
};

#undef ZC_API