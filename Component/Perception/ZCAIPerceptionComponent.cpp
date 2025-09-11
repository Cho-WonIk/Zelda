// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Perception/ZCAIPerceptionComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"

#include "Development/ZCLogger.h"

#include "Development/ZCDebug.h"
#if !UE_BUILD_SHIPPING
#include "AIController.h"
namespace Zelda::Debug::AI
{
	static bool bDrawDebugAll = false;
	static bool bDrawPerception = false;
	static FAutoConsoleVariableRef CVar_DebugAll(Zelda::Debug::AI::all, bDrawDebugAll, TEXT("AI 디버깅 전체 On/Off"), ECVF_Default);
	static FAutoConsoleVariableRef CVar_DebugPerception(Zelda::Debug::AI::perception, bDrawPerception, TEXT("AI 디버깅 감지 On/Off"), ECVF_Default);
}
#endif

UZCAIPerceptionComponent::UZCAIPerceptionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.016f;
	bWantsInitializeComponent = true;

	OnTargetPerceptionUpdated.AddDynamic(this, &UZCAIPerceptionComponent::TargetPerceptionUpdated);
}

void UZCAIPerceptionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	for (auto& Config : SensesConfig)
	{
		if (auto* Sight = Cast<UAISenseConfig_Sight>(Config))
		{
			SightConfig = Sight;
			Sight->SetMaxAge(PerceptionConfig.SightConfig.MaxLifeTime);
			ConfigureSense(*Sight);
		}
		else if (auto* Hearing = Cast<UAISenseConfig_Hearing>(Config))
		{
			HearingConfig = Hearing;
			Hearing->SetMaxAge(PerceptionConfig.HearingConfig.MaxLifeTime);
			ConfigureSense(*Hearing);
		}
		else if (auto* Damage = Cast<UAISenseConfig_Damage>(Config))
		{
			DamageConfig = Damage;
			Damage->SetMaxAge(PerceptionConfig.DamageConfig.MaxLifeTime);
			ConfigureSense(*Damage);
		}
	}
}

void UZCAIPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAggro(DeltaTime);
}

void UZCAIPerceptionComponent::ResetAggro()
{
	if (CurrentAggro > 0.0f) { OnAggroZero.Broadcast(); }
	CurrentAggro = 0.0f;

	OnAggroUpdate.Broadcast(AggroThreshold > 0.0f ? CurrentAggro / AggroThreshold : 0.0f);
}

void UZCAIPerceptionComponent::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const bool bSensed = Stimulus.WasSuccessfullySensed();

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		PerceptionData.SightInfo.bIsDetected = bSensed;

		if (bSensed)
		{
			PerceptionData.SightInfo.SensedActor = Actor;
			PerceptionData.SightInfo.SensedLocation = Stimulus.StimulusLocation;
			PerceptionData.SightInfo.SensedTime = Stimulus.GetAge();

		}
		// 시야 어그로 증가 로직은 Tick에서 계산
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		//PerceptionData.bIsHearing = bSensed;
		PerceptionData.HearingInfo.bIsDetected = bSensed;

		if (bSensed)
		{
			PerceptionData.HearingInfo.SensedActor = Actor;
			PerceptionData.HearingInfo.SensedLocation = Stimulus.StimulusLocation;
			PerceptionData.HearingInfo.SensedTime = Stimulus.GetAge();

			const FVector HearLocation = Stimulus.StimulusLocation;
			const float Distance = FVector::Dist(GetBodyActor()->GetActorLocation(), HearLocation);
			const float AggroStrength = CalculateAggroStrength(Distance, HearingConfig->HearingRange);
			AddAggro(AggroStrength, PerceptionConfig.HearingConfig.AggroIncrement, 1.0f);
		}
		else
		{
			PerceptionData.HearingInfo.SensedActor = nullptr;
		}
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		//PerceptionData.bIsTakeDamage = bSensed;
		PerceptionData.DamageInfo.bIsDetected = bSensed;

		if (bSensed)
		{
			PerceptionData.DamageInfo.SensedActor = Actor;
			PerceptionData.DamageInfo.SensedLocation = Stimulus.StimulusLocation;
			PerceptionData.DamageInfo.SensedTime = Stimulus.GetAge();

			AddAggro(1.0f, PerceptionConfig.DamageConfig.AggroIncrement, 1.0f);
		}
		else
		{
			PerceptionData.DamageInfo.SensedActor = nullptr;
		}
	}
}

void UZCAIPerceptionComponent::UpdateAggro(float DeltaTime)
{
	// 시각 어그로 수치 증가
	TArray<AActor*> SightPerceptionActors;
	GetCurrentlyPerceivedActors(SightConfig->GetSenseImplementation(), SightPerceptionActors);

	if (PerceptionData.SightInfo.bIsDetected)
	{
		float Distance = FLT_MAX;

		PerceptionData.ClosestSightActor = UGameplayStatics::FindNearestActor(GetBodyActor()->GetActorLocation(), SightPerceptionActors, Distance);

		const float AggroStrength = CalculateAggroStrength(Distance, SightConfig->LoseSightRadius);

		AddAggro(AggroStrength, PerceptionConfig.SightConfig.AggroIncrement, DeltaTime);
	}
	else
	{
		PerceptionData.ClosestSightActor = nullptr;
		PerceptionData.SightInfo.SensedActor = nullptr;
	}

	//UZCLogger::Warning(TEXT("현재 가장 가까운 시야 감지 액터 : {0}, 가장 최근에 감지된 액터 : {1}"), PerceptionData.ClosestSightActor, PerceptionData.SightInfo.SensedActor);

	// 어그로 수치 감소, 모든 감지가 안걸렸을 때만
	// 배틀 상태 진입시 일정 시간 지연 후 어그로 감소 시작

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// 감지가 완전히 끊겼을 경우
	const bool bNoSense = !PerceptionData.SightInfo.bIsDetected && !PerceptionData.HearingInfo.bIsDetected && !PerceptionData.DamageInfo.bIsDetected;

	// 감지가 끊긴 첫 순간만 기록
	if (bNoSense)
	{
		if (LastSensedLostTime < 0.0f)
		{
			LastSensedLostTime = CurrentTime;
		}
	}
	else
	{
		// 감지가 있는 상태이면 초기화
		LastSensedLostTime = -1.0f;
	}

	if (CurrentAggro > 0.0f && bNoSense)
	{
		if (LastSensedLostTime >= 0.0f && (CurrentTime - LastSensedLostTime) >= AggroDecrementDelay)
		{
			if (!PerceptionData.SightInfo.bIsDetected)
			{
				SubtractAggro(1.0f, PerceptionConfig.SightConfig.AggroDecrement, DeltaTime);
			}
			if (!PerceptionData.HearingInfo.bIsDetected)
			{
				SubtractAggro(1.0f, PerceptionConfig.HearingConfig.AggroDecrement, DeltaTime);
			}
			if (!PerceptionData.DamageInfo.bIsDetected)
			{
				SubtractAggro(1.0f, PerceptionConfig.DamageConfig.AggroDecrement, DeltaTime);
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (Zelda::Debug::AI::bDrawDebugAll || Zelda::Debug::AI::bDrawPerception)
	{
		FVector PawnLocation = GetBodyActor()->GetActorLocation();

		DrawDebugString(GetWorld(), PawnLocation + FVector(0, 0, 100),
			FString::Printf(TEXT("현재 어그로 수치: %.2f"), CurrentAggro),
			nullptr, FColor::Red, 0.016f, true);

		DrawDebugString(GetWorld(), PawnLocation + FVector(0, 0, 70),
			FString::Printf(TEXT("시각 감지 : %s, 청각 감지 : %s, 데미지 감지 : %s"),
				PerceptionData.SightInfo.bIsDetected ? TEXT("O") : TEXT("X"),
				PerceptionData.HearingInfo.bIsDetected ? TEXT("O") : TEXT("X"),
				PerceptionData.DamageInfo.bIsDetected ? TEXT("O") : TEXT("X")),
			nullptr, FColor::Red, 0.016f, true);
	}
#endif
}

float UZCAIPerceptionComponent::AddAggro(float Strength, float InCrement, float DeltaTime)
{
	CurrentAggro = FMath::Clamp(CurrentAggro + (Strength * InCrement * DeltaTime), 0.0f, AggroThreshold);

	OnAggroUpdate.Broadcast(AggroThreshold > 0.0f ? CurrentAggro / AggroThreshold : 0.0f);

	return CurrentAggro;
}

float UZCAIPerceptionComponent::SubtractAggro(float Strength, float Decrement, float DeltaTime)
{
	CurrentAggro = FMath::Clamp(CurrentAggro - (Strength * Decrement * DeltaTime), 0.0f, CurrentAggro);

	OnAggroUpdate.Broadcast(AggroThreshold > 0.0f ? CurrentAggro / AggroThreshold : 0.0f);

	if (FMath::IsNearlyZero(CurrentAggro)) { OnAggroZero.Broadcast(); }

	return CurrentAggro;
}

float UZCAIPerceptionComponent::CalculateAggroStrength(float Distance, float MaxDistance)
{
	if (Distance >= MaxDistance) return 0.0f;

	// 거리 비율을 구해 1에서 빼면 어그로 비율
	const float Pct = FMath::GetRangePct(0.0f, MaxDistance, Distance);
	return FMath::Clamp(1.0f - Pct, 0.0f, 1.0f);
}
