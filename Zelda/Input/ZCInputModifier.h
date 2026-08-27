// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "ZCInputModifier.generated.h"

/**
 * 비트부호가 1이면 1.0f를 반환하는 Modifier
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "ZC Sign Only"))
class ZELDA_API UZCInputModifierSignOnly : public UInputModifier
{
	GENERATED_BODY()

protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;
};

/**
* Threshold보다 값이 작으면 0 Threshold이상이면 1을 반환하는 Modifier
*/
UCLASS(NotBlueprintable, meta = (DisplayName = "ZC Threshold"))
class ZELDA_API UZCInputModifierThreshold : public UInputModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	float Threshold = 0.5f;

protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;
};

/**
* 일정시간마다 값을 반환하는 Modifier
*/
UCLASS(NotBlueprintable, meta = (DisplayName = "ZC Interval"))
class ZELDA_API UZCInputModifierInterval : public UInputModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	float IntervalTime = 0.1f;
	
protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

private:
	float TimeRemaining = 0.0f;
	static FInputActionValue MakeZero(EInputActionValueType Type);
};

/**
* 입력값이 반대방향이면 값을 반환하는 Modifier
*/
UCLASS(NotBlueprintable, meta = (DisplayName = "ZC Pulse"))
class ZELDA_API UZCInputModifierPulse : public UInputModifier
{
	GENERATED_BODY()

public:
	/** 비교 기준이 되는 방향 (정규화된 벡터여야 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pulse")
	FVector BaseDirection = FVector::ForwardVector;

	/** 반응할 각도(기본: 100도 이상이면 반대 방향으로 판단) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pulse")
	float ThresholdAngleDeg = 100.f;

protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;
};