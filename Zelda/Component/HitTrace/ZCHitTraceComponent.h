// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Struct/ZCHitTraceStruct.h"

#include "ZCHitTraceComponent.generated.h"

/*
 * HitTraceComponent
 * - HitTraceStruct : HitTrace에 필요한 데이터 구조체
 * - HitActorDelegate : HitActor에 대한 Delegate
 * - BeginTrace : HitTrace 시작
 * - EndTrace : HitTrace 종료
 * - TryTrace : HitTrace 시도
 * - MakeBoxTrace : Box Trace 생성
 * - CheckActorIsFirstTrace : Actor가 처음으로 Hit된 것인지 확인
 * 
 bool bAllowMultipleHitActor는 단일 공격 중 여러번 피격 판정 발생 여부를 결정하는 변수
 HitInterval은 단일 공격 중 여러번 피격 판정 발생 시간 간격 함수

*/

DECLARE_DELEGATE_TwoParams(FOnHitActorDelegate, AActor*, const FHitResult&);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCHitTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZCHitTraceComponent();

public:
	FOnHitActorDelegate OnHitActor;

public:
	void BeginTrace(const FName& TaskName, const FZCHitTraceStruct& HitTraceStruct);
	void EndTrace(const FName& TaskName);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	void TryTrace(const FName& TaskName);
	void MakeBoxTrace(const FName& TaskName, const FVector& Start, const FVector& End, const FVector& HalfSize, const FRotator& Orientation);
	bool CheckActorIsFirstTrace(AActor* Actor, const FName& TaskName);

private:
	TMap<FName, FZCHitTraceStruct> TraceDataMap;
	TMap<FName, FVector> LastStartLocationMap;
	TMap<FName, FVector> LastEndLocationMap;
	TMap<FName, TMap<TWeakObjectPtr<AActor>, float>> LastHitTimeMap;
};
