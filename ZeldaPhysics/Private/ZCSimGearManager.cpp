// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCSimGearManager.h"
#include "GameFramework/HUD.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"

#include "Runtime/ZCGearSettings.h"
#include "Core/ZCSimGearGraph.h"
#include "Core/ZCSimGearAsyncCallback.h"
#include "Runtime/Gear/ZCGearMovementComponent.h"

TMap<FPhysScene*, FZCSimGearManager*> FZCSimGearManager::SceneToGearManagerMap;

FDelegateHandle FZCSimGearManager::OnPostWorldInitializationHandle;
FDelegateHandle FZCSimGearManager::OnWorldCleanupHandle;

bool FZCSimGearManager::GInitialized;

FZCSimGearManager::FZCSimGearManager(FPhysScene* PhysScene) : Scene(*PhysScene)
{
	check(PhysScene);

	if (!GInitialized)
	{
		GInitialized = true;
		
		// PhysScene->GetOwningWorld()는 항상 이시점에서 nullptr이다.
		// 그래서 델리게이트를 통해 콜백함수에서 등록을 한다.
		OnPostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddStatic(&FZCSimGearManager::OnPostWorldInitialization);
		OnWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&FZCSimGearManager::OnWorldCleanup);

		if (!IsRunningDedicatedServer())
		{
			AHUD::OnShowDebugInfo.AddStatic(&FZCSimGearManager::OnShowDebugInfo);
		}
	}

	// 같은 PhysScene으로 두번 등록되는 것을 방지
	ensure(FZCSimGearManager::SceneToGearManagerMap.Find(PhysScene) == nullptr);

	// 현재 매니저에 물리 씬을 등록
	FZCSimGearManager::SceneToGearManagerMap.Add(PhysScene, this);

	SimGearGraph = MakeUnique<ZCGear::FZCSimGearGraph>();
	SimGearGraphBuffer = MakeUnique<ZCGear::FZCSimGearGraphBuffer>();
	SimGearInputContainer = MakeUnique<ZCGear::FZCSimGearInputBufferContainer>();
}

FZCSimGearManager::~FZCSimGearManager()
{
	if (SceneToGearManagerMap.Contains(&Scene))
	{
		SceneToGearManagerMap.Remove(&Scene);
	}
}

void FZCSimGearManager::OnPostWorldInitialization(UWorld* InWorld, const UWorld::InitializationValues)
{
	FZCSimGearManager* Manager = FZCSimGearManager::GetManagerFromScene(InWorld->GetPhysicsScene());
	if (Manager)
	{
		Manager->RegisterCallbacks(InWorld);
	}
}

void FZCSimGearManager::OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources)
{
	FZCSimGearManager* Manager = FZCSimGearManager::GetManagerFromScene(InWorld->GetPhysicsScene());
	if (Manager)
	{
		Manager->UnregisterCallbacks();
	}
}

void FZCSimGearManager::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{

}

FZCSimGearManager* FZCSimGearManager::GetManagerFromScene(FPhysScene* PhysScene)
{
	FZCSimGearManager** MangerPtr = SceneToGearManagerMap.Find(PhysScene);
	if (!MangerPtr) return nullptr;
	return *MangerPtr;
}

void FZCSimGearManager::AddGearSimulation(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation)
{
	if (!InGearSimulation.IsValid()) return;
	SimGearGraphBuffer->AddNode(InGearSimulation.Get());
}

void FZCSimGearManager::RemoveGearSimulation(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation)
{
	if (!InGearSimulation.IsValid()) return;
	SimGearGraphBuffer->RemoveNode(InGearSimulation.Get());
}

void FZCSimGearManager::SetGearSimulationEnabled(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation, bool bEnabled)
{
	if (!InGearSimulation.IsValid()) return;
	SimGearGraphBuffer->ChangeNodeState(InGearSimulation.Get(), bEnabled ? EZCGearState::Enabled : EZCGearState::Disabled);
}

void FZCSimGearManager::RegisterPlayerInputGearSimulation(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation)
{
	if (!InGearSimulation.IsValid()) return;
	SimGearGraphBuffer->RegisterApplyPlayerInputGear(InGearSimulation.Get());

	//InGearSimulation.Get()->GearSimulationPT->SetAcceptingPlayerInput(true);
}

void FZCSimGearManager::UnRegisterPlayerInputGearSimulation(TWeakObjectPtr<UZCGearMovementComponent> InGearSimulation)
{
	if (!InGearSimulation.IsValid()) return;
	SimGearGraphBuffer->UnRegisterApplyPlayerInputGear(InGearSimulation.Get());

	//InGearSimulation.Get()->GearSimulationPT->SetAcceptingPlayerInput(false);
}

void FZCSimGearManager::PlayerInput(const FVector2D& InMoveInput)
{
	for (const auto& [Type, GearInputModifier] : GearInputModifierMap)
	{
		GearInputModifier->ModifyInput(InMoveInput, *SimGearInputContainer.Get());
	}
}

void FZCSimGearManager::SetCenterOfMassOffset(const FVector& InCenterOfMassOffset, const float& InMass)
{
	if (SimGearInputContainer.IsValid())
	{
		SimGearInputContainer->SetCenterOfMassOffset(InCenterOfMassOffset);
		SimGearInputContainer->SetTotalMass(InMass);
	}
}

void FZCSimGearManager::SetPlayerViewVector(const FVector& InViewVector)
{
	if (SimGearInputContainer.IsValid())
	{
		SimGearInputContainer->SetPlayerViewDirection(InViewVector);
	}
}

void FZCSimGearManager::RegisterCallbacks(UWorld* InWorld)
{
	OnPhysScenePreTickHandle = Scene.OnPhysScenePreTick.AddRaw(this, &FZCSimGearManager::Update);
	OnPhysScenePostTickHandle = Scene.OnPhysScenePostTick.AddRaw(this, &FZCSimGearManager::PostUpdate);

	check(AsyncCallback == nullptr);
	AsyncCallback = Scene.GetSolver()->CreateAndRegisterSimCallbackObject_External<FZCSimGearAsyncCallback>();

	InitializeGearInputModifiers();

}

void FZCSimGearManager::UnregisterCallbacks()
{
	Scene.OnPhysScenePreTick.Remove(OnPhysScenePreTickHandle);
	Scene.OnPhysScenePostTick.Remove(OnPhysScenePostTickHandle);

	if (AsyncCallback)
	{
		Scene.GetSolver()->UnregisterAndFreeSimCallbackObject_External(AsyncCallback);
		AsyncCallback = nullptr;
	}

	DeinitializeGearInputModifiers();
}

void FZCSimGearManager::InitializeGearInputModifiers()
{
	const UZCGearSettings* GearSettings = GetDefault<UZCGearSettings>();

	GearInputModifierMap.Empty();

	for(const TSoftObjectPtr<UZCGearInputModifier>& ModifierPtr : GearSettings->WheelGearInputModifiers)
	{
		UZCGearInputModifier* LoadedModifier = ModifierPtr.LoadSynchronous();

		if (LoadedModifier)
		{
			EZCGearType ModifierType = LoadedModifier->Type;

			if (!GearInputModifierMap.Contains(ModifierType))
			{
				GearInputModifierMap.Add(ModifierType, TStrongObjectPtr<UZCGearInputModifier>(LoadedModifier));
			}

		}
	}
}

void FZCSimGearManager::DeinitializeGearInputModifiers()
{
	GearInputModifierMap.Empty();
}

void FZCSimGearManager::ScenePreTick(FPhysScene* PhysScene, float DeltaTime)
{
	if (SimGearGraph.IsValid() && SimGearGraphBuffer.IsValid())
	{
		SimGearGraph->ApplyBuffer(*SimGearGraphBuffer);

	}
}

void FZCSimGearManager::Update(FPhysScene* PhysScene, float DeltaTime)
{
	UWorld* World = Scene.GetOwningWorld();
	SubStepCount = 0;
	ScenePreTick(PhysScene, DeltaTime);

	ParallelUpdateGear(DeltaTime);

	if (World)
	{
		FZCSimGearManagerAsyncInput* AsyncInput = AsyncCallback->GetProducerInputData_External();



		// 반복문으로 데이터 받아옴
		// 각 기어에 대한 최종 입력데이터 처리
		const TArray<UZCGearMovementComponent*>& ActiveGears = SimGearGraph->GetCachedActiveGears();
		for (UZCGearMovementComponent* Gear : ActiveGears)
		{
			Gear->Update(DeltaTime);
			Gear->FinalizeSimCallbackData(*AsyncInput);
		}

		// 입력 버퍼에서 모듈 입력 데이터 추출 및 물리 스레드로 전달
		PrepareModuleInputs(AsyncInput);
	}
}

void FZCSimGearManager::PostUpdate(FChaosScene* PhysScene)
{
}

void FZCSimGearManager::ProcessPlayerInput(float DeltaTime)
{
	if (!SimGearInputContainer.IsValid()) return;
}

void FZCSimGearManager::PrepareModuleInputs(FZCSimGearManagerAsyncInput* AsyncInput)
{
	if (!AsyncInput) return;
	if (!SimGearInputContainer.IsValid()) return;

	// 입력 버퍼 컨테이너에서 공유 모듈 입력 컨테이너로 추출
	AsyncInput->PrepareSharedInputs(*SimGearInputContainer.Get());

	// 물리 스레드로 전달 완료 후 입력 버퍼 비우기
	SimGearInputContainer->ClearBuffers();
}

void FZCSimGearManager::ParallelUpdateGear(float DeltaTime)
{
	FZCSimGearManagerAsyncInput* AsyncInput = AsyncCallback->GetProducerInputData_External();

	AsyncInput->Reset();

	const TArray<UZCGearMovementComponent*>& ActiveGears = SimGearGraph->GetCachedActiveGears();

	// 예상되는 공간을 미리 할당
	AsyncInput->GearInputs.Reserve(ActiveGears.Num());
	AsyncInput->TimeStamp = TimeStamp;
	AsyncInput->World = Scene.GetOwningWorld();

	// 모든	출력을 처리하기 위해 미래의 출력도 포함하여 가져옴
	Chaos::TSimCallbackOutputHandle<FZCSimGearManagerAsyncOutput> AsyncOutputLatest;
	while (AsyncOutputLatest = AsyncCallback->PopFutureOutputData_External())
	{
		PendingOutputs.Emplace(MoveTemp(AsyncOutputLatest));
	}

	// 지금 계산하는 물리량들은 시간이 흐르기전의 상태를 기준으로 함
	const float ResultsTime = AsyncCallback->GetSolver()->GetPhysicsResultsTime_External();

	// 아직 처리되지 않는(미래의) 첫번째 데이터를 찾음
	int32 LastOutputIndex = 0;
	for (; LastOutputIndex < PendingOutputs.Num(); ++LastOutputIndex)
	{
		if (PendingOutputs[LastOutputIndex]->InternalTime > ResultsTime) break;
	}

	// 이전에 처리된 가장 최근의 출력 데이터를 저장(보간 처리에 활용)
	if (LastOutputIndex > 0)
	{
		LatestOutput = MoveTemp(PendingOutputs[LastOutputIndex - 1]);
	}

	// 소비된 모든 출력 제거
	TArray<Chaos::TSimCallbackOutputHandle<FZCSimGearManagerAsyncOutput>> NewPendingOutputs;
	for (int32 Index = LastOutputIndex; Index < PendingOutputs.Num(); ++Index)
	{
		NewPendingOutputs.Emplace(MoveTemp(PendingOutputs[Index]));
	}
	PendingOutputs = MoveTemp(NewPendingOutputs);

	if (UWorld* World = Scene.GetOwningWorld())
	{
		int32 NumGearsInActiveBatch = 0;

		auto NextOutput = PendingOutputs.Num() > 0 ? PendingOutputs[0].Get() : nullptr;
		float Alpha = 0.0f;

		if (NextOutput && LatestOutput)
		{
			const float Denom = NextOutput->InternalTime - LatestOutput->InternalTime;
			if (Denom > KINDA_SMALL_NUMBER)
			{
				Alpha = (ResultsTime - LatestOutput->InternalTime) / Denom;
			}
		}

		// 비동기 입력 생성 로직
		for (UZCGearMovementComponent* Gear : ActiveGears)
		{
			AsyncInput->GearInputs.Add(Gear->SetCurrentAsyncData(AsyncInput->GearInputs.Num(), LatestOutput.Get(), NextOutput, Alpha, TimeStamp));
		}

	}

	++TimeStamp;

	if (ActiveGears.Num() > 0)
	{
		ParallelFor(ActiveGears.Num(), [&](int32 Index)
		{
			ActiveGears[Index]->ParalleUpdate(DeltaTime);
		});
	}
}
