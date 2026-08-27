// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/Gear/ZCGearMovementComponent.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "PhysicsProxy/ClusterUnionPhysicsProxy.h"
#include "Physics/Experimental/PhysScene_Chaos.h"

#include "PBDRigidsSolver.h"

#include "ZCSimGearManager.h"
#include "Core/Module/ZCSimGearBaseModule.h"
#include "Core/ZCSimGearAsyncCallback.h"

#include "Runtime/Gear/ZCGearBaseComponent.h"

UZCGearMovementComponent::UZCGearMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	
}

void UZCGearMovementComponent::OnCreatePhysicsState()
{
	Super::OnCreatePhysicsState();

	CreateGearSimulation();

	ActorsToIgnore.Add(GetOwner());
}

void UZCGearMovementComponent::OnDestroyPhysicsState()
{
	DestroyGearSimulation();

	Super::OnDestroyPhysicsState();
}

void UZCGearMovementComponent::EnableSimulation()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	FPhysScene* PhysScene = World->GetPhysicsScene();

	if (FZCSimGearManager* SimManager = FZCSimGearManager::GetManagerFromScene(PhysScene))
	{
		UE_LOG(LogTemp, Warning, TEXT("시뮬레이션 자체 활성화 등록"));
		SimManager->SetGearSimulationEnabled(this, true);
	}
}

void UZCGearMovementComponent::DisableSimulation()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	FPhysScene* PhysScene = World->GetPhysicsScene();

	if (FZCSimGearManager* SimManager = FZCSimGearManager::GetManagerFromScene(PhysScene))
	{
		UE_LOG(LogTemp, Warning, TEXT("시뮬레이션 자체 비활성화 등록"));
		SimManager->SetGearSimulationEnabled(this, false);
	}
}

// 물리 스레드에서 처리된 값을 병렬로 가져옴
void UZCGearMovementComponent::ParalleUpdate(float DeltaTime)
{
	if (FZCSimGearAsyncOutput* CurrentOutput = static_cast<FZCSimGearAsyncOutput*>(CurrentAsyncOutput))
	{
		
	}
}

void UZCGearMovementComponent::Update(float DeltaTime)
{
	if (CurrentAsyncInput && GetPhysicsProxy())
	{
		CurrentAsyncInput->Proxy = GetPhysicsProxy();

		FZCSimGearAsyncInput* AsyncInput = static_cast<FZCSimGearAsyncInput*>(CurrentAsyncInput);

	}
}

void UZCGearMovementComponent::FinalizeSimCallbackData(FZCSimGearManagerAsyncInput& Input)
{
	CurrentAsyncInput = nullptr;
	CurrentAsyncOutput = nullptr;
}

void UZCGearMovementComponent::CreateGearModules()
{
	TArray<ZCGear::IZCSimGearBaseModule*> SimModules;

	CreateGearModulesRecursively(UpdatedComponent, SimModules);

	if (!GearSimulationPT.IsValid()) return;

	GearSimulationPT->Initialize(MoveTemp(SimModules));
	SetGearState(GearState);

	IPhysicsProxyBase* Proxy = GetPhysicsProxy();
	FZCSimGearSimulation* Simulation = GearSimulationPT.Get();
	EnqueueToPhysicsThread([Simulation, Proxy]()
	{
		Simulation->CacheRootParticle(Proxy);
	});
}

TUniquePtr<FZCSimGearAsyncInput> UZCGearMovementComponent::SetCurrentAsyncData(int32 InputIdx, FZCSimGearManagerAsyncOutput* CurrentOutput, FZCSimGearManagerAsyncOutput* NextOutput, float Alpha, int32 ManagerTimeStamp)
{
	TUniquePtr<FZCSimGearAsyncInput> CurrentInput = MakeUnique<FZCSimGearAsyncInput>();

	ensure(CurrentAsyncInput == nullptr);
	ensure(CurrentAsyncOutput == nullptr);

	CurrentAsyncInput = CurrentInput.Get();
	CurrentAsyncInput->SetGear(this);
	NextAsyncOutput = nullptr;
	OutputInterAlpha = 0.0f;

	CurrentAsyncInput->Proxy = GetPhysicsProxy();

	if (CurrentOutput && InputIdx < CurrentOutput->GearOutputs.Num())
	{
		if (FZCSimGearAsyncOutput* Output = CurrentOutput->GearOutputs[InputIdx].Get())
		{
			if (Output->bValid)
			{
				CurrentAsyncOutput = Output;

				if (NextOutput && InputIdx < NextOutput->GearOutputs.Num())
				{
					NextAsyncOutput = NextOutput->GearOutputs[InputIdx].Get();
					OutputInterAlpha = Alpha;
				}
			}
		}
	}

	return CurrentInput;
}

void UZCGearMovementComponent::SetGearState(EZCGearState NewState)
{
	GearState = NewState;
	
	GearSimulationPT->SetCurrentGearState(NewState);
}

void UZCGearMovementComponent::CreateGearSimulation()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	GearSimulationPT = MakeUnique<FZCSimGearSimulation>();

	FPhysScene* PhysScene = World->GetPhysicsScene();

	if (FZCSimGearManager* SimManager = FZCSimGearManager::GetManagerFromScene(PhysScene))
	{
		SimManager->AddGearSimulation(this);
	}
}

void UZCGearMovementComponent::DestroyGearSimulation()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	FPhysScene* PhysScene = World->GetPhysicsScene();
	check(PhysScene);

	if (!PhysScene->GetSolver()) return;

	if (!GearSimulationPT.IsValid()) return;

	if (FZCSimGearManager* SimManager = FZCSimGearManager::GetManagerFromScene(PhysScene))
	{
		SimManager->RemoveGearSimulation(this);
	}

	GearSimulationPT->Terminate();
	GearSimulationPT.Reset(nullptr);
}

void UZCGearMovementComponent::CreateGearModulesRecursively(USceneComponent* ParentComponent, TArray<ZCGear::IZCSimGearBaseModule*>& OutModules)
{
	if (!ParentComponent) return;

	if (UZCGearBaseComponent* GearComponent = Cast<UZCGearBaseComponent>(ParentComponent))
	{
		ZCGear::IZCSimGearBaseModule* NewModule = GearComponent->CreateNewGearModule();
		if (NewModule)
		{
			NewModule->SetOwnerSimulation(GearSimulationPT.Get());
			IPhysicsProxyBase* Proxy = GearComponent->GetCurrentParticleHandle();
			NewModule->SetPhysicsProxy(Proxy);
			NewModule->OnConstruction_External(GetPhysicsObject(Proxy));
			OutModules.Add(NewModule);
		}
	}

	const TArray<USceneComponent*>& ChildComponents = ParentComponent->GetAttachChildren();
	for (USceneComponent* ChildComponent : ChildComponents)
	{
		CreateGearModulesRecursively(ChildComponent, OutModules);
	}
}

IPhysicsProxyBase* UZCGearMovementComponent::GetPhysicsProxy()
{
	if (CachedPhysicsProxy) return CachedPhysicsProxy;

	if (const FBodyInstance* BodyInstance = UpdatedPrimitive ? UpdatedPrimitive->GetBodyInstance() : nullptr)
	{
		return CachedPhysicsProxy = BodyInstance->GetPhysicsActor();
	}

	return nullptr;
}

Chaos::FPhysicsObjectHandle UZCGearMovementComponent::GetPhysicsObject(IPhysicsProxyBase* Proxy)
{
	Chaos::EnsureIsInGameThreadContext();

	Chaos::FPhysicsObjectHandle PhysicsObject = nullptr;

	if (!Proxy) return nullptr;

	switch (Proxy->GetType())
	{
	case EPhysicsProxyType::ClusterUnionProxy:
	{
		if (Chaos::FClusterUnionPhysicsProxy* CUProxy = static_cast<Chaos::FClusterUnionPhysicsProxy*>(Proxy))
			return CUProxy->GetPhysicsObjectHandle();
	}
		break;

	case EPhysicsProxyType::SingleParticleProxy:
	{
		if (Chaos::FSingleParticlePhysicsProxy* ParticleProxy = static_cast<Chaos::FSingleParticlePhysicsProxy*>(Proxy))
			return ParticleProxy->GetPhysicsObject();
	}
		break;
	}

	return Chaos::FPhysicsObjectHandle();
}

void UZCGearMovementComponent::EnqueueToPhysicsThread(TFunction<void()> Command)
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	FPhysScene* PhysScene = World->GetPhysicsScene();
	if (!PhysScene) return;

	Chaos::FPhysicsSolver* Solver = PhysScene->GetSolver();
	if (!Solver) return;

	Solver->EnqueueCommandImmediate(MoveTemp(Command));
}
