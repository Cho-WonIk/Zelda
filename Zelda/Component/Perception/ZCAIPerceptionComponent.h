// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameData/Struct/ZCPerceptionConfig.h"
#include "ZCAIPerceptionComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAggroUpdateDelegate, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FonAggroThresholdDelegate);

UCLASS()
class ZELDA_API UZCAIPerceptionComponent : public UAIPerceptionComponent
{
	GENERATED_BODY()
	
public:
	UZCAIPerceptionComponent();

public:
	// 어그로 업데이트 델리게이트
	FOnAggroUpdateDelegate OnAggroUpdate;

	// 어그로 수치가 0이되면 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Perception|Delegate")
	FonAggroThresholdDelegate OnAggroZero;

	// 어그로 수치가 임계값을 초과하면 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Perception|Delegate")
	FonAggroThresholdDelegate OnAggroMax;

	// 어그로 강제 리셋 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Perception|Delegate")
	FonAggroThresholdDelegate OnAggroReset;

protected:
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 어그로 임계값
	float GetAggroThreshold() { return AggroThreshold; }

	// 배틀 상태시 어그로 감소 지연 시간
	float GetAggroDecrementDelay() { return AggroDecrementDelay; }

	// 어그로 강제 리셋 시간
	float GetAggroResetTime() { return AggroResetTime; }

	// 현재 어그로 수치 가져오기
	float GetCurrentAggro() const { return CurrentAggro; }

	// 어그로 수치 초기화
	UFUNCTION(BlueprintCallable, Category = "Perception", meta = (DisplayName = "어그로 초기화"))
	void ResetAggro();

	// 몬스터의 감각 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Perception", meta = (DisplayName = "감각 데이터 가져오기"))
	const FPerceptionData& GetPerceptionData() const { return PerceptionData; }

	UFUNCTION(BlueprintCallable, Category = "Percpetion|Sight", meta = (DisplayName = "시각 감각 데이터 가져오기"))
	const FSenseDetectionInfo& GetSightData() const { return PerceptionData.SightInfo; }

	UFUNCTION(BlueprintCallable, Category = "Percpetion|Hearing", meta = (DisplayName = "청각 감각 데이터 가져오기"))
	const FSenseDetectionInfo& GetHearingData() const { return PerceptionData.HearingInfo; }

	UFUNCTION(BlueprintCallable, Category = "Percpetion|Damage", meta = (DisplayName = "데미지 감각 데이터 가져오기"))
	const FSenseDetectionInfo& GetDamageData() const { return PerceptionData.DamageInfo; }

protected:
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	void UpdateAggro(float DeltaTime);

protected:
	// 몬스터의 시각 감지 설정
	TObjectPtr<class UAISenseConfig_Sight> SightConfig;
	// 몬스터의 청각 감지 설정
	TObjectPtr<class UAISenseConfig_Hearing> HearingConfig;
	// 몬스터의 데미지 감지 설정
	TObjectPtr<class UAISenseConfig_Damage> DamageConfig;

protected:
	// 몬스터의 감각 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config", meta = (DisplayName = "감각 평가 설정"))
	struct FZCPerceptionConfig PerceptionConfig;

protected:
	// 어그로 임계값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config", meta = (DisplayName = "어그로 임계값"))
	float AggroThreshold = 0.0f;

	// 어그로 감소 지연 시간 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config", meta = (DisplayName = "어그로 감소 지연 시간"))
	float AggroDecrementDelay = 0.0f;

	// 어그로 강제 리셋 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config", meta = (DisplayName = "어그로 리셋 시간"))
	float AggroResetTime = 0.0f;

	// 감지가 사라진 시점 기록
	float LastSensedLostTime = -1.0f;

	// 현재 어그로 수치
	float CurrentAggro = 0.0f;

	// 현재 몬스터의 감각 감지 데이터
	FPerceptionData PerceptionData;

private:
	// 어그로 수치 덧셈함수
	float AddAggro(float Strength, float InCrement, float DeltaTime);
	// 어그로 수치 뺄셈함수
	float SubtractAggro(float Strength, float Decrement, float DeltaTime);
	// 어그로 강도 계산함수
	static float CalculateAggroStrength(float Distance, float MaxDistance);
};
