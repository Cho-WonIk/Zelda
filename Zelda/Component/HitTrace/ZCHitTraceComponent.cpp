// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/HitTrace/ZCHitTraceComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Development/ZCDebug.h"

#if !UE_BUILD_SHIPPING
namespace Zelda::Debug::HitTrace
{
	static bool bDrawDebugVar = false;

	static FAutoConsoleVariableRef CVar_DrawDebug(
		Zelda::Debug::HitTrace::DrawDebug,
		bDrawDebugVar,
		TEXT("공격의 히트 판정 범위 시각화"),
		ECVF_Default
	);
}
#endif

// Sets default values for this component's properties
UZCHitTraceComponent::UZCHitTraceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // 초기에는 Tick을 사용하지 않음

	// ...
}

void UZCHitTraceComponent::BeginTrace(const FName& TaskName, const FZCHitTraceStruct& HitTraceStruct)
{
	TraceDataMap.FindOrAdd(TaskName) = HitTraceStruct;

	SetComponentTickEnabled(true);
}

void UZCHitTraceComponent::EndTrace(const FName& TaskName)
{
	TraceDataMap.Remove(TaskName);
	LastStartLocationMap.Remove(TaskName);
	LastEndLocationMap.Remove(TaskName);
	LastHitTimeMap.Remove(TaskName);

	if (TraceDataMap.IsEmpty())
	{
		SetComponentTickEnabled(false);
	}
}

// Called every frame
void UZCHitTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 추적할려는 모든 트레이스 실행
	for (const TPair<FName, FZCHitTraceStruct>& Pair : TraceDataMap)
	{
		TryTrace(Pair.Key);
	}
}

void UZCHitTraceComponent::TryTrace(const FName& TaskName)
{
	FZCHitTraceStruct* LocalTraceData = TraceDataMap.Find(TaskName);

	if (LocalTraceData)
	{
		// 시작과 끝 소켓 위치 계산
		const FVector StartLocation = LocalTraceData->Mesh->GetSocketLocation(LocalTraceData->BeginSocket);
		const FVector EndLocation = LocalTraceData->Mesh->GetSocketLocation(LocalTraceData->EndSocket);

		// 이전 프레임의 위치
		const FVector LastStartLocation = LastStartLocationMap.Find(TaskName) ? *LastStartLocationMap.Find(TaskName) : FVector::ZeroVector;
		const FVector LastEndLocation = LastEndLocationMap.Find(TaskName) ? *LastEndLocationMap.Find(TaskName) : FVector::ZeroVector;

		// 시작과 끝 소켓 위치가 같으면 트레이스 하지 않고 초기화
		if ((LastStartLocation + LastEndLocation) == FVector(0.0f, 0.0f, 0.0f))
		{
			LastStartLocationMap.FindOrAdd(TaskName) = StartLocation;
			LastEndLocationMap.FindOrAdd(TaskName) = EndLocation;
			return;
		}

		// 트레이스할 박스 크기 계산
		FVector HalfSize;
		HalfSize.X = FVector::Dist(LastStartLocation, StartLocation) / 2.0f;
		HalfSize.Y = LocalTraceData->Radius;
		HalfSize.Z = LocalTraceData->Radius;

		// 트레이스 실행
		MakeBoxTrace(
			TaskName,
			(StartLocation + LastStartLocation) / 2.0f,
			(EndLocation + LastEndLocation) / 2.0f,
			HalfSize,
			UKismetMathLibrary::FindLookAtRotation(LastStartLocation, StartLocation)
		);

		// 현재 위치 저장
		LastStartLocationMap.FindOrAdd(TaskName) = StartLocation;
		LastEndLocationMap.FindOrAdd(TaskName) = EndLocation;
	}
}

void UZCHitTraceComponent::MakeBoxTrace(const FName& TaskName, const FVector& Start, const FVector& End, const FVector& HalfSize, const FRotator& Orientation)
{
	FZCHitTraceStruct* LocalTraceData = TraceDataMap.Find(TaskName);
	TArray<FHitResult> OutHits;

#if !UE_BUILD_SHIPPING
	if (Zelda::Debug::HitTrace::bDrawDebugVar)
	{
		//LocalTraceData->DrawDebugType = EDrawDebugTrace::ForDuration;
	}
#endif

	// 트레이스할 타입에 따라 다른 트레이스 함수 호출
	switch (LocalTraceData->TraceType)
	{
	case EHitTraceType::Channel:
		UKismetSystemLibrary::BoxTraceMulti(this, Start, End, HalfSize, Orientation,
			LocalTraceData->TraceChannel, LocalTraceData->TraceComplex, LocalTraceData->IgnoreActors, LocalTraceData->DrawDebugType, OutHits, LocalTraceData->bIgnoreSelf,
			FLinearColor::Red, FLinearColor::Green,
			5.0f
		);
		break;
	case EHitTraceType::ObjectType:
		UKismetSystemLibrary::BoxTraceMultiForObjects(this, Start, End, HalfSize, Orientation,
			LocalTraceData->ObjectTypes, LocalTraceData->TraceComplex, LocalTraceData->IgnoreActors, LocalTraceData->DrawDebugType, OutHits, LocalTraceData->bIgnoreSelf,
			FLinearColor::Red, FLinearColor::Green,
			5.0f
		);
		break;
	case EHitTraceType::ProfileName:
		UKismetSystemLibrary::BoxTraceMultiByProfile(this, Start, End, HalfSize, Orientation,
			LocalTraceData->ProfileName, LocalTraceData->TraceComplex, LocalTraceData->IgnoreActors, LocalTraceData->DrawDebugType, OutHits, LocalTraceData->bIgnoreSelf,
			FLinearColor::Red, FLinearColor::Green,
			5.0f
		);
		break;
	}

	for (auto &OutHit : OutHits)
	{
		AActor* HitActor = OutHit.GetActor();
		if (HitActor)
		{
			// 중복 검사
			if (CheckActorIsFirstTrace(HitActor, TaskName))
			{
				// 데미지 이벤트 브로드캐스팅

				OnHitActor.ExecuteIfBound(HitActor, OutHit);
			}
		}
	}
}

bool UZCHitTraceComponent::CheckActorIsFirstTrace(AActor* Actor, const FName& TaskName)
{
	if (Actor)
	{
		FZCHitTraceStruct* LocalTraceData = TraceDataMap.Find(TaskName);
		if (LocalTraceData)
		{
			// 다단 히트가 아닌 경우
			if (!LocalTraceData->bAllowMultipleHit)
			{
				TSet<TWeakObjectPtr<AActor>>& LocalSet = LocalTraceData->TracedActors;
				TWeakObjectPtr<AActor> WeakActor(Actor);

				if (!LocalSet.Contains(WeakActor))
				{
					LocalSet.Add(WeakActor);
					return true;
				}
			}
			// 다단 히트가 허용된 경우
			else
			{
				const float* LastTime = LastHitTimeMap.FindOrAdd(TaskName).Find(Actor);
				const float CurrentTime = GetWorld()->GetTimeSeconds();

				if (!LastTime || (CurrentTime - *LastTime) >= LocalTraceData->HitInterval)
				{
					LastHitTimeMap[TaskName].Add(Actor, CurrentTime);
					return true;
				}
			}

		}
	}
	return false;
}

