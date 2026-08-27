// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/Input/ZCGearInputModifier.h"

float FZCGearModifierConfig::CalculateModifiedValue(const float& RawInput) const
{
	if (FMath::Abs(RawInput) <= KINDA_SMALL_NUMBER) return 0.0f;

	const float Sign = FMath::Sign(RawInput);
	const float AbsInput = FMath::Abs(RawInput);
	float Result = 0.0f;

	switch (ModifierType)
	{
	case EZCGearInputModifierType::LinearInterpolation:
		Result = AbsInput;
		break;

	case EZCGearInputModifierType::SquaredInterpolation:
		// f(x) = x^2
		Result = AbsInput * AbsInput;
		break;

	case EZCGearInputModifierType::CustomInterpolation:
		if (CustomCurve)
		{
			Result = CustomCurve->GetFloatValue(AbsInput);
		}
		else
		{
			Result = AbsInput;
		}
		break;
	}

	return Sign * (Scale * Result + Bias);
}


void UZCGearInputModifier::ModifyInput(const FVector2D& RawInput, ZCGear::FZCSimGearInputBufferContainer& GearInputContainer)
{
	// X축 변조
	//UE_LOG(LogTemp, Warning, TEXT("X축 변조"));
	CalculateModifiedInput(RawInput.X, XAxisModifiers, GearInputContainer.AccessModuleInputSet(Type));
	// Y축 변조
	//UE_LOG(LogTemp, Warning, TEXT("Y축 변조"));
	CalculateModifiedInput(RawInput.Y, YAxisModifiers, GearInputContainer.AccessModuleInputSet(Type));
	// X축 동기화
	if (XSyncType == EZCGearInputSyncType::Sync)
	{
		CalculateYSyncInput(RawInput.X, XSyncModifier, GearInputContainer.AccessModuleInputYSyncValues(Type));
	}
}

void UZCGearInputModifier::CalculateModifiedInput(const float& RawInput, const TMap<EZCGearAxisType, FZCGearModifierConfig>& InAxisModifier, ZCGear::FZCSimGearModuleSet& OutModule)
{
	for (const auto& [AxisType, AxisModifier] : InAxisModifier)
	{
		if (AxisType == EZCGearAxisType::None || AxisType == EZCGearAxisType::Max) continue;

		CalculateAxisInput(RawInput, AxisModifier, OutModule[AxisType]);
	}
}

void UZCGearInputModifier::CalculateYSyncInput(const float& RawYInput, const FZCGearModifierConfig& InYSyncModifier, ZCGear::FZCSimGearValue& OutY)
{
	const float ModifiedValue = InYSyncModifier.CalculateModifiedValue(RawYInput);

	OutY.ApplyType = InYSyncModifier.ApplyType;
	OutY.Value = ModifiedValue;
}

void UZCGearInputModifier::CalculateAxisInput(const float& RawInput, const FZCGearModifierConfig& AxisModifier, ZCGear::FZCSimGearValue& OutAxisBuffer)
{
	const float ModifiedValue = AxisModifier.CalculateModifiedValue(RawInput);

	OutAxisBuffer.ApplyType = AxisModifier.ApplyType;
	OutAxisBuffer.Value = ModifiedValue;
}

