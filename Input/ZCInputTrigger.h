// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "ZCInputTrigger.generated.h"

class UEnhancedPlayerInput;
class UInputAction;

// ChordActions의 InputAction이 하나이상 Trigger되어야 Trigger반환, 이외의 반환값은 없음
UCLASS(NotBlueprintable, meta = (DisplayName = "ZC Chorded Action OR", NotInputConfigurable = "true"))
class ZELDA_API UZCInputTriggerChordActionOR : public UInputTrigger
{
	GENERATED_BODY()
	
public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	// Implicit, so action cannot fire unless this is firing.
	virtual ETriggerType GetTriggerType_Implementation() const override { return ETriggerType::Implicit; }

	virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;
	virtual bool IsBlocking(const ETriggerState State) const override { return State != ETriggerState::Triggered; }

public:
	// Array of actions where at least one must be triggering for this trigger's action to trigger
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Trigger Settings", meta = (DisplayThumbnail = "false"))
	TArray<TObjectPtr<const UInputAction>> ChordActions;
};

/*==================================================================*/

/*
* ChordAction의 ETriggerEvent값이 Trigger인 경우에 Ongoing혹은 Trigger가 발생
* ChordAction이 Trigger가 처음으로 발생하면 Start
* Threshold이하로 값이 지속되면 Ongoing
* Threshold이상로 값이 지속되면 Trigger
* Ongoing상태에서 ChordAction이 !Trigger상태가 되면 Canceled호출
* Trigger상태에서 ChordAction이 Trigger상태라면 Complete호출
*/

UCLASS(NotBlueprintable, meta = (DisplayName = "ZC Chorded Action"))
class ZELDA_API UZCInputActionTriggerChordAction : public UInputTrigger
{
	GENERATED_BODY()

public:
	virtual ETriggerEventsSupported GetSupportedTriggerEvents() const override { return ETriggerEventsSupported::All; }

protected:
	// Chord가 Trigger되지 않으면 발동 불가능
	virtual ETriggerType GetTriggerType_Implementation() const override { return ETriggerType::Implicit; }

	virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;
	//virtual bool IsBlocking(const ETriggerState State) const override { return State != ETriggerState::Triggered; }

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Trigger Settings", meta = (DisplayThumbnail = "false"))
	TObjectPtr<const UInputAction> ChordAction = nullptr;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Trigger Settings")
	float Threshold = 1.0f;

private:
	// ModifiedValue 변화 감지
	FInputActionValue LastValue;

	// Start가 트리거되었는지 확인
	bool bHasLastValue = false;

	// hold지속시간 저장
	float ChangingElapsed = 0.0f;

	// 값 비교 유틸
	bool HasValueChanged(const FInputActionValue& A, const FInputActionValue& B, double Eps = 1e-6) const;
	void ResetState();
};
