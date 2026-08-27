// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameData/Enum/ZCMonster.h"
#include "GameData/Struct/ZCPerceptionConfig.h"
#include "ZCAIControllerBase.generated.h"

struct FAIStimulus;
class UZCPathFollowingComponent;
class UZCStateTreeAIComponent;

UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle			UMETA(DisplayName = "대기"),
	Search			UMETA(DisplayName = "탐색"),
	InBattle		UMETA(DisplayName = "전투"),
	None			UMETA(DisplayName = "없음"),
};

UCLASS()
class ZELDA_API AZCAIControllerBase : public AAIController
{
	GENERATED_BODY()
	
public:
	AZCAIControllerBase(const FObjectInitializer& ObjectInitializer);

	/*======= 오버라이드 함수 =======*/
public:
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override; // 아군, 적군, 중립 구분 로직
protected:
	virtual void OnPossess(APawn* InPawn) override;

	/*====== Getter 함수 =======*/
public:
	UFUNCTION(BlueprintCallable, Category = "Component")
	UZCPathFollowingComponent* GetZCPathFollowingComponent() const { return ZCPathFollowingComponent; } // 경로 추적 컴포넌트 가져오기

	UFUNCTION(BlueprintCallable, Category = "Component")
	UZCAIPerceptionComponent* GetZCPerceptionComponent() const { return ZCPerceptionComponent; } // 감지 컴포넌트 가져오기

	UFUNCTION(BlueprintCallable, Category = "Component")
	UStateTreeAIComponent* GetStateTreeAIComponent() const { return StateTreeAIComponent; } // 상태 트리 AI 컴포넌트 가져오기

protected:
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<class UStateTreeAIComponent> StateTreeAIComponent;

	class UZCPathFollowingComponent* ZCPathFollowingComponent = nullptr; // 경로 추적 컴포넌트

	class UZCAIPerceptionComponent* ZCPerceptionComponent = nullptr; // 감지 컴포넌트
};
