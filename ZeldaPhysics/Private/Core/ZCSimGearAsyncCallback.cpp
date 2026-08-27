// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ZCSimGearAsyncCallback.h"
#include "PBDRigidsSolver.h"

#include "Runtime/Gear/ZCGearMovementComponent.h"

TUniquePtr<struct FZCSimGearAsyncOutput> FZCSimGearAsyncInput::Simulate(UWorld* World, const float DeltaSeconds, const float TotalSeconds) const
{
	TUniquePtr<FZCSimGearAsyncOutput> Output = MakeUnique<FZCSimGearAsyncOutput>();

	// 프록시가 없는 경우 빈값을 반환
	if (!Proxy) return Output;

	if (Gear && Gear->GearSimulationPT)
	{
		Gear->GearSimulationPT->Simulate(DeltaSeconds, *this, *Output);

		FZCSimGearAsyncOutput& OutputData = *Output.Get();

		// Output 데이터 여기서 채움
	}

	Output->bValid = true;

	return MoveTemp(Output);
}

void FZCSimGearAsyncInput::ApplyDeferredForces() const
{
	// 지연된 힘 적용 로직
	if (Gear && Proxy && Gear->GearSimulationPT)
	{
		Gear->GearSimulationPT->ApplyDeferredForces(Proxy);
	}
}

void FZCSimGearAsyncInput::ProcessInputs()
{

}

bool FZCSimGearAsyncInput::IsAcceptingPlayerInput() const
{
	if (Gear && Gear->GearSimulationPT)
	{
		return Gear->GearSimulationPT->IsAcceptingPlayerInput();
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FZCSimGearManagerAsyncInput::PrepareSharedInputs(const ZCGear::FZCSimGearInputBufferContainer& InputBuffer)
{
	// 입력 버퍼에서 공유 모듈 입력 컨테이너로 추출
	SharedModuleInputContainer.ExtractFromInputBuffer(InputBuffer);
	bInputsProcessed = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FName FZCSimGearAsyncCallback::GetFNameForStatId() const
{
	const static FLazyName StaticName("FZCSimGearAsyncCallback");
	return StaticName;
}

// 물리 스레드에서 호출되는 입력처리
void FZCSimGearAsyncCallback::ProcessInputs_Internal(int32 PhysicsStep)
{
	const FZCSimGearManagerAsyncInput* AsyncInput = GetConsumerInput_Internal();
	if (!AsyncInput) return;

	for (const TUniquePtr<FZCSimGearAsyncInput>& GearInput : AsyncInput->GearInputs)
	{
		if (GearInput.IsValid())
		{
			
		}
	}
}

// 물리 스레드에서 호출
void FZCSimGearAsyncCallback::OnPreSimulate_Internal()
{
	float DeltaTime = GetDeltaTime_Internal();
	float SimTime = GetSimTime_Internal();

	const FZCSimGearManagerAsyncInput* Input = GetConsumerInput_Internal();
	if (!Input) return;

	const int32 NumGears = Input->GearInputs.Num();

	UWorld* World = Input->World.Get();
	if (!World || NumGears == 0) return;

	Chaos::FPhysicsSolver* PhysicsSolver = static_cast<Chaos::FPhysicsSolver*>(GetSolver());
	
	FZCSimGearManagerAsyncOutput& Output = GetProducerOutputData_Internal();
	Output.GearOutputs.AddDefaulted(NumGears);
	Output.Timestamp = Input->TimeStamp;

	const TArray<TUniquePtr<FZCSimGearAsyncInput>>& GearInputsBatch = Input->GearInputs;
	TArray<TUniquePtr<FZCSimGearAsyncOutput>>& GearOutputsBatch = Output.GearOutputs;

	// 전역 입력 컨테이너 포인터 (블랙보드)
	const ZCGear::FZCSimGearModuleInputContainer* SharedInputPtr = &Input->SharedModuleInputContainer;

	auto LambdaParallelUpdate = [World, DeltaTime, SimTime, SharedInputPtr, &GearInputsBatch, &GearOutputsBatch](int32 Index)
		{
			const FZCSimGearAsyncInput& GearInput = *GearInputsBatch[Index];

			if (!GearInput.Proxy) return;

			if (GearInput.IsAcceptingPlayerInput())
			{
				GearInput.GearPhysicsInputData.SharedModuleInputs = SharedInputPtr;
			}
			else
			{
				GearInput.GearPhysicsInputData.SharedModuleInputs = nullptr;
			}

			bool bWake = false;
			GearOutputsBatch[Index] = GearInput.Simulate(World, DeltaTime, SimTime);

		};
	Chaos::PhysicsParallelFor(GearOutputsBatch.Num(), LambdaParallelUpdate);

	// 모든 기어의 입력 처리 완료 후 지연된 힘 적용
	for (const TUniquePtr<FZCSimGearAsyncInput>& GearInput : GearInputsBatch)
	{
		if (GearInput.IsValid())
		{
			// 지연된 힘
			GearInput->ApplyDeferredForces();
		}
	}
}

void FZCSimGearAsyncCallback::OnContactModification_Internal(Chaos::FCollisionContactModifier& Modifications)
{
}

