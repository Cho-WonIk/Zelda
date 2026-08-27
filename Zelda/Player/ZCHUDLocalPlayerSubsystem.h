// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Settings/UI/ZCDefaultWidgetSettings.h"
#include "GameData/Enum/ZCWidget.h"
#include "GameData/Struct/ZCHUDStruct.h"
#include "ZCHUDLocalPlayerSubsystem.generated.h"

class UZCInGameHUDWidget;
class AZCPlayerCharacter;
class AZCPlayerController;

enum ECustomMovementMode : uint8;
enum class ESkillType : uint8;
enum class EItemType : uint8;

enum class EZCCrosshairState : uint8
{
	None,       // 비활성화
	Normal,     // 활성화 - 일반
	LockedOn    // 활성화 - 락온
};

struct FZCHUDState
{
	EZCHUDMode HUDMode = EZCHUDMode::Normal;

// 일반 상태
	struct FNormalState
	{
		FTEvent<EZCFaceButtonEvent> FaceButtonEvent;
		EMovementMode Movement;
		ECustomMovementMode CustomMovement;
	};

// 스킬 상태
	struct FSkillState
	{
		ESkillType SkillType;
		FTEvent<EFaceButtonType> FaceButtonType;
	};

// 인디케이터
	struct FIndicator
	{
		int32 Main = INDEX_NONE;
		int32 Sub = INDEX_NONE;
	};

	EZCCrosshairState CrosshairState = EZCCrosshairState::None;

	struct FNormalState Normal;
	struct FSkillState Skill;

	struct FIndicator SkillIndicator;
};

UCLASS()
class ZELDA_API UZCHUDLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	static UZCHUDLocalPlayerSubsystem& Get(const UObject* WorldContext);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const;

protected:
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController);

	UFUNCTION()
	void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void OnMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode);

	UFUNCTION()
	void OnCanClimbDownCliff(const bool bDownCliff);

public:
	EZCFaceButtonEvent GetCurrentFaceButtonEvent()
	{
		if (HUDState.HUDMode == EZCHUDMode::Skill || HUDState.HUDMode == EZCHUDMode::Gear) return EZCFaceButtonEvent::None;
		return HUDState.Normal.FaceButtonEvent.GetPriority();
	}

	void SetHUDModeState(EZCHUDMode NewMode);

	// 페이스 버튼 관련 함수들
public:
	// 노멀 상태의 FaceButton에 표시될 정보를 입력합니다.
	void AddFaceButtonContext(EZCFaceButtonEvent AddButtonEvent, EFaceButtonType AddFaceButton,  const FText& Label);
	
	// 스킬 상태의 스킬 별 FaceButton에 표시될 정보를 입력합니다.
	void AddSkillButtonContext(ESkillType SkillType, EFaceButtonType AddFaceButton, const FText& Label);

	/*페이스 버튼 관련 함수들*/
	void ShowFaceButtonEvent(EZCFaceButtonEvent ShowButtonEvent);
	void HideFaceButtonEvent(EZCFaceButtonEvent HideButtonEvent);
	void ResetFaceButtonEvent();
	/*스킬 상태 페이스 버튼 관련 함수들*/
	void ShowSkillButtonEvent(ESkillType SkillType, EFaceButtonType ShowFaceButton);
	void HideSkillButtonEvent(ESkillType SkillType, EFaceButtonType HideFaceButton);
	void ResetSkillButtonEvent();

	// 숏컷 관련 함수들
public:
	void ShowShortCutUI(const FVector2D& InputDirection);
	void HideShortCutUI();
	void ScrollShortCutItemList(float ScrollValue);

	bool GetCurrentShortCutSelection(int32& OutIdx, EItemType& OutType) const;

	// 인디케이터 관련 함수들
public:
	void ShowSkillIndicator(ESkillType SkillType, int32 StateIdx);

	// 크로스헤어 관련 함수들
public:
	void ShowCrosshair();
	void HideCrosshair();

	void SetCrosshairLockOn(bool bLocked);

private:
	int32 GetMainIndicatorIndexFromSkill(ESkillType SkillType) const;
	int32 GetSubIndicatorIndexFromSkillState(ESkillType SkillType, int32 StateIndex) const;
	int32 GetFaceButtonModeIndexFromSkill(ESkillType SkillType) const;

	void UpdateFaceButton();
	void UpdateIndicator();
	void UpdateCrosshair();

private:
	UPROPERTY()
	TObjectPtr<class UZCInGameHUDWidget> InGameWidget;

	AZCPlayerCharacter* PlayerCharacter = nullptr;
	AZCPlayerController* PlayerController = nullptr;

	FZCHUDState HUDState;

	EItemType CurrentShortCutType;
	
	TMap<EZCFaceButtonEvent, FZCFaceButtonContext> FaceButtonMap;
	TMap<ESkillType, TMap<EFaceButtonType, FText>> SkillContextMap;
};
