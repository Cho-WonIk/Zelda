// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/ZCInputModifier.h"

FInputActionValue UZCInputModifierSignOnly::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	const EInputActionValueType ValueType = CurrentValue.GetValueType();

	switch (ValueType)
	{
	case EInputActionValueType::Boolean:
	{
		bool bValue = CurrentValue.Get<bool>();
		return FInputActionValue(bValue ? 1.0f : 0.0f);
	}
	case EInputActionValueType::Axis1D:
	{
		float Value = CurrentValue.Get<float>();
		float SignValue = (Value > 0.0f) ? 0.0f : 1.0f;
		return FInputActionValue(SignValue);
	}
	case EInputActionValueType::Axis2D:
	{
		FVector2D Value = CurrentValue.Get<FVector2D>();
		FVector2D SignValue;
		SignValue.X = (Value.X > 0.0f) ? 0.0f : 1.0f;
		SignValue.Y = (Value.Y > 0.0f) ? 0.0f : 1.0f;
		return FInputActionValue(SignValue);
	}
	case EInputActionValueType::Axis3D:
	{
		FVector Value = CurrentValue.Get<FVector>();
		FVector SignValue;
		SignValue.X = (Value.X > 0.0f) ? 0.0f : 1.0f;
		SignValue.Y = (Value.Y > 0.0f) ? 0.0f : 1.0f;
		SignValue.Z = (Value.Z > 0.0f) ? 0.0f : 1.0f;
		return FInputActionValue(SignValue);
	}
	default:
		return CurrentValue;
	}
}

FInputActionValue UZCInputModifierThreshold::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	const EInputActionValueType ValueType = CurrentValue.GetValueType();

	switch (CurrentValue.GetValueType())
	{
	case EInputActionValueType::Boolean:
	{
		const bool b = CurrentValue.Get<bool>();
		return FInputActionValue(b ? 1.0f : 0.0f);
	}
	case EInputActionValueType::Axis1D:
	{
		const float v = CurrentValue.Get<float>();
		if (FMath::Abs(v) < Threshold) return FInputActionValue(0.0f);
		return FInputActionValue(FMath::Sign(v));
	}
	case EInputActionValueType::Axis2D:
	{
		const FVector2D v = CurrentValue.Get<FVector2D>();
		FVector2D out(0.f, 0.f);
		if (FMath::Abs(v.X) >= Threshold) out.X = FMath::Sign(v.X);
		if (FMath::Abs(v.Y) >= Threshold) out.Y = FMath::Sign(v.Y);
		return FInputActionValue(out);
	}
	case EInputActionValueType::Axis3D:
	{
		const FVector v = CurrentValue.Get<FVector>();
		FVector out(0.f, 0.f, 0.f);
		if (FMath::Abs(v.X) >= Threshold) out.X = FMath::Sign(v.X);
		if (FMath::Abs(v.Y) >= Threshold) out.Y = FMath::Sign(v.Y);
		if (FMath::Abs(v.Z) >= Threshold) out.Z = FMath::Sign(v.Z);
		return FInputActionValue(out);
	}
	default:
		return CurrentValue;
	}
}

FInputActionValue UZCInputModifierInterval::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	TimeRemaining += DeltaTime;

	if (TimeRemaining >= IntervalTime)
	{
		TimeRemaining = 0.0f;
		return CurrentValue;
	}

	return MakeZero(CurrentValue.GetValueType());
}

FInputActionValue UZCInputModifierInterval::MakeZero(EInputActionValueType Type)
{
	switch (Type)
	{
	case EInputActionValueType::Boolean: return FInputActionValue(false);
	case EInputActionValueType::Axis1D:  return FInputActionValue(0.f);
	case EInputActionValueType::Axis2D:  return FInputActionValue(FVector2D::ZeroVector);
	case EInputActionValueType::Axis3D:  return FInputActionValue(FVector::ZeroVector);
	default:                             return FInputActionValue(0.f);
	}
}

FInputActionValue UZCInputModifierPulse::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	const FVector Value = CurrentValue.Get<FVector>();
	if (Value.IsNearlyZero()) return FVector::ZeroVector;

	// 정규화
	FVector InputDir = Value.GetSafeNormal();
	FVector BaseDir = BaseDirection.GetSafeNormal();

	// 두 벡터의 내적(dot)
	float Dot = FVector::DotProduct(InputDir, BaseDir);

	// 내적으로 각도 계산
	float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));

	// 조건 만족 시 Pulse = 1, 아니면 0
	float PulseValue = (AngleDeg >= ThresholdAngleDeg) ? 1.f : 0.f;

	return FVector(PulseValue, 0.f, 0.f);
}
