// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/ZCInputTrigger.h"
#include "EnhancedInputModule.h"
#include "EnhancedPlayerInput.h"
#include "HAL/IConsoleManager.h"
#include "Misc/DataValidation.h"

#include "Development/ZCLogger.h"

#define LOCTEXT_NAMESPACE "ZCInputTriggers"

#if WITH_EDITOR
EDataValidationResult UZCInputTriggerChordActionOR::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (ChordActions.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("NullChordedAction", "A valid action is required for the ZC Chorded Action input trigger!"));
	}

	for (auto &ChordAction : ChordActions)
	{
		if (!ChordAction)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(LOCTEXT("NullChordedAction", "A valid action is required for the ZC Chorded Action input trigger!"));
		}
	}

	return Result;
}
#endif

ETriggerState UZCInputTriggerChordActionOR::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime)
{
	for (auto &ChordAction : ChordActions)
	{
		const FInputActionInstance* EventData = PlayerInput->FindActionInstanceData(ChordAction);

		if (!EventData) continue;

		if (EventData->GetTriggerEvent() == ETriggerEvent::Triggered) return ETriggerState::Triggered;
	}
	return ETriggerState::None;
}

/*===================================================================================================================================*/

ETriggerState UZCInputActionTriggerChordAction::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime)
{
	if (!ChordAction)
	{
		ResetState();
		return ETriggerState::None;
	}

	const FInputActionInstance* EventData = PlayerInput->FindActionInstanceData(ChordAction);
	if (!EventData) 
	{
		ResetState();
		return ETriggerState::None;
	}

	const ETriggerEvent CurrentChordEvent = EventData->GetTriggerEvent();

	if (CurrentChordEvent == ETriggerEvent::Canceled || CurrentChordEvent == ETriggerEvent::Completed)
	{
		ResetState();
		return ETriggerState::None;
	}

	if (CurrentChordEvent != ETriggerEvent::Triggered)
	{
		ResetState();
		return ETriggerState::None;
	}

	bool bChanged = false;
	// ChordAction의 Trigger상태가 처음되었을 경우
	if (!bHasLastValue)
	{
		bChanged = true;
		bHasLastValue = true;
		LastValue = ModifiedValue;
	}
	else
	{
		// Last Input과 비교해서 값이 변화했는지 확인
		bChanged = HasValueChanged(ModifiedValue, LastValue);
		// 값이 변화했으면 새롭게 갱신
		if (bChanged)
		{
			ChangingElapsed = 0.0f;
			LastValue = ModifiedValue;
		}
	}

	// 값이 새롭게 변했거나, 이전 값이 누적 중이라면
	if (bChanged || ChangingElapsed > 0.0f)
	{
		ChangingElapsed += DeltaTime;

		if (ChangingElapsed >= Threshold)
		{
			ChangingElapsed = 0.0f;
			return ETriggerState::Triggered;
		}
		return ETriggerState::Ongoing;
	}
	return ETriggerState::None;
}

bool UZCInputActionTriggerChordAction::HasValueChanged(const FInputActionValue& A, const FInputActionValue& B, double Eps) const
{
	return FMath::Abs(A.GetMagnitude() - B.GetMagnitude()) > Eps;
}

void UZCInputActionTriggerChordAction::ResetState()
{
	ChangingElapsed = 0.0F;
	bHasLastValue = false;
}

/*===================================================================================================================================*/

UZCInputTriggerShake::UZCInputTriggerShake()
{

}

ETriggerState UZCInputTriggerShake::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime)
{
	const float CurrentTime = PlayerInput->GetWorld()->GetTimeSeconds();

	// 쿨다운 중이면 아무 입력도 트리거시키지 않음
	if (CurrentTime - LastTriggeredTime < CooldownSeconds)
	{
		// 에너지는 감소만 진행
		CurrentShakeEnergy -= (DecayRate * DeltaTime);
		CurrentShakeEnergy = FMath::Max(0.0f, CurrentShakeEnergy);
		return ETriggerState::None;
	}

	const FVector Value = ModifiedValue.Get<FVector>();
	float CurrentSpeed = Value.Size2D();

	// 에너지 누적
	CurrentShakeEnergy += CurrentSpeed;

	// 에너지 자연 감소
	CurrentShakeEnergy -= (DecayRate * DeltaTime);
	CurrentShakeEnergy = FMath::Max(0.0f, CurrentShakeEnergy);

	// 판정 로직
	if (CurrentShakeEnergy > ShakeThreshold)
	{
		// 트리거 발동
		CurrentShakeEnergy = 0.0f;
		LastTriggeredTime = CurrentTime; // 쿨다운 시작
		return ETriggerState::Triggered;
	}

	return CurrentShakeEnergy > 0.0f ? ETriggerState::Ongoing : ETriggerState::None;
}
