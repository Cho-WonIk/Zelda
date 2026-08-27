// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeExecutionContext.h"

#include "AI/ZCAIControllerBase.h"
#include "Character/Monster/ZCMonsterCharacter.h"
#include "GameData/Struct/ZCPerceptionConfig.h"
#include "ZCAIPerceptionSTE.generated.h"

class UZCAIPerceptionComponent;
class AZCWeaponActor;
class AZCShieldActor;

// Instance Data 구조체
USTRUCT()
struct ZELDA_API FZCAIPerceptionSTEInstanceData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AZCAIControllerBase> AIController = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AZCMonsterCharacter> MonsterCharacter = nullptr;

	// 어그로 수치 임계값
	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "어그로 임계값"))
	float AggroThreshold = 0.0f;

	// 배틀 상태시 어그로 감소 지연 시간 설정
	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "어그로 감소 시작 시간"))
	float AggroDecrementDelay = 0.0f;

	// 어그로 강제 리셋 시간
	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "어그로 강제 리셋 시간"))
	float AggroResetTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "현재 무기"))
	TObjectPtr<AZCWeaponActor> CurrentWeapon = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "현재 방패"))
	TObjectPtr<AZCShieldActor> CurrentShield = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "감각 데이터"))
	FPerceptionData SenseDetectionInfo;

	// 퍼셉션 감지 여부
	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "퍼셉션 감지"))
	uint8 bIsPerception : 1;

	// 마지막 감지 이후 경과 시간
	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "마지막 감지 이후 경과 시간"))
	float TimeSinceLastPerception = 0.0f;

	// 현재 어그로 수치
	UPROPERTY(VisibleAnywhere, Category = "Output", meta = (DisplayName = "어그로 수치"))
	float CurrentAggro = 0.0f;

	// 내부 사용 데이터 (StateTree에서 노출되지 않음)
	UPROPERTY()
	TObjectPtr<UZCAIPerceptionComponent> AIPerceptionComponent = nullptr;

	FZCAIPerceptionSTEInstanceData()
	{
		bIsPerception = false;
	}
};

// StateTree Evaluator 구조체
USTRUCT(meta = (DisplayName = "ZC 퍼셉션 감각 평가"))
struct ZELDA_API FZCAIPerceptionSTE : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZCAIPerceptionSTEInstanceData;

	FZCAIPerceptionSTE() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FName GetIconName() const override;
	virtual FColor GetIconColor() const override;
#endif
};