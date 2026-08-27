// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Runtime/ZCGearType.h"
#include "Core/ZCSimGearInput.h"

#include "ZCGearInputModifier.generated.h"

USTRUCT(BlueprintType)
struct FZCGearModifierConfig
{
	GENERATED_BODY()

public:
	// 사용할 보간 방식
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "사용할 보간 방식"))
	EZCGearInputModifierType ModifierType = EZCGearInputModifierType::LinearInterpolation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "적용 방법"))
	EZCGearInputApplyType ApplyType = EZCGearInputApplyType::None;

	// 수식의 계수 (a)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "계수(a)"))
	float Scale = 1.0f;

	// 수식의 편차 (b)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "편차(b)"))
	float Bias = 0.0f;

	// CustomInterpolation일 때 참조할 커브 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "커스텀 커브", EditCondition = "ModifierType == EZCGearInputModifierType::CustomInterpolation"))
	TObjectPtr<UCurveFloat> CustomCurve;

public:
	// 입력값은 원점 기준으로 대칭되어 나타남.
	// ex) RawInput = 0.5 -> ModifiedValue = f(0.5)
	//    RawInput = -0.5 -> ModifiedValue = -f(0.5)
	float CalculateModifiedValue(const float& RawInput) const;
};

/** 기어 타입별 입력 수정 방식
 * 
 */
UCLASS()
class ZELDAPHYSICS_API UZCGearInputModifier : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type", meta = (DisplayName = "입력 종류"))
	EZCGearType Type = EZCGearType::Undefined;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "입력 : X축 -> 변조될 축 및 변조 종류"))
	TMap< EZCGearAxisType, FZCGearModifierConfig> XAxisModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (DisplayName = "입력 : Y축 -> 변조될 축 및 변조 종류"))
	TMap< EZCGearAxisType, FZCGearModifierConfig> YAxisModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "X Sync", meta = (DisplayName = "X축 방향 동기화 여부"))
	EZCGearInputSyncType XSyncType = EZCGearInputSyncType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "X Sync", meta = (DisplayName = "X축 방향 동기화 수치", EditCondition = "XSyncType == EZCGearInputSyncType::Sync"))
	FZCGearModifierConfig XSyncModifier;

public:
	void ModifyInput(const FVector2D& RawInput, ZCGear::FZCSimGearInputBufferContainer& GearInputContainer);

protected:
	void CalculateModifiedInput(const float& RawInput, const TMap< EZCGearAxisType, FZCGearModifierConfig>& InAxisModifier, ZCGear::FZCSimGearModuleSet& OutModule);
	void CalculateYSyncInput(const float& RawYInput, const FZCGearModifierConfig& InYSyncModifier , ZCGear::FZCSimGearValue& OutY);

	void CalculateAxisInput(const float& RawInput, const FZCGearModifierConfig& AxisModifier, ZCGear::FZCSimGearValue& OutAxisBuffer);
};
