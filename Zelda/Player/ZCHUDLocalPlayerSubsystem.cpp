// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ZCHUDLocalPlayerSubsystem.h"
#include "UI/HUD/ZCHUDWidget.h"
#include "Component/Movement/ZCCharacterMovementComponent.h"
#include "Character/Player/ZCPlayerCharacter.h"
#include "Player/ZCPlayerController.h"
#include "Player/ZCPlayerState.h"
#include "Component/Inventory/ZCInventoryComponent.h"
#include "Component/Device/ZCDeviceEnum.h"
#include "GameData/Enum/ZCItemType.h"
#include "Development/ZCLogger.h"

UZCHUDLocalPlayerSubsystem& UZCHUDLocalPlayerSubsystem::Get(const UObject* WorldContext)
{
	const ULocalPlayer* LocalPlayer = WorldContext->GetWorld()->GetFirstLocalPlayerFromController();
	check(LocalPlayer);

	UZCHUDLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UZCHUDLocalPlayerSubsystem>();
	check(Subsystem);

	return *Subsystem;
}

void UZCHUDLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UZCHUDLocalPlayerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UZCHUDLocalPlayerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UZCHUDLocalPlayerSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	if (!NewPlayerController || !NewPlayerController->IsLocalController()) return;

	const UZCDefaultWidgetSettings* UIDefault = GetDefault<UZCDefaultWidgetSettings>();

	InGameWidget = CreateWidget<UZCInGameHUDWidget>(NewPlayerController, UIDefault->ZCInGameHUDClass);
	if (InGameWidget)
	{
		InGameWidget->AddToViewport(0);
	}

	NewPlayerController->OnPossessedPawnChanged.AddDynamic(this, &UZCHUDLocalPlayerSubsystem::OnPawnChanged);

	PlayerController = Cast<AZCPlayerController>(NewPlayerController);

}

void UZCHUDLocalPlayerSubsystem::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	PlayerCharacter = Cast<AZCPlayerCharacter>(NewPawn);
	if (PlayerCharacter)
	{
		if (UZCCharacterMovementComponent* MovementComp = PlayerCharacter->GetZCCharacterMovementComponent())
		{
			MovementComp->OnMovementChangeDelegate.AddDynamic(this, &UZCHUDLocalPlayerSubsystem::OnMovementChange);
			MovementComp->OnCanClimbDownCliffDelegate.BindUObject(this, &UZCHUDLocalPlayerSubsystem::OnCanClimbDownCliff);
		}
	}
}

void UZCHUDLocalPlayerSubsystem::OnMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode)
{
	const bool bIsMovementModeChanged = (HUDState.Normal.Movement != NewMovementMode);
	const bool bIsCustomMovementModeChanged = (HUDState.Normal.CustomMovement != NewCustomMovementMode);

	bool bIsChange = bIsMovementModeChanged || bIsCustomMovementModeChanged;

	HUDState.Normal.Movement = NewMovementMode;
	HUDState.Normal.CustomMovement = NewCustomMovementMode;

	if (!bIsChange) return;

	if (NewMovementMode != MOVE_Walking)
	{
		HideFaceButtonEvent(EZCFaceButtonEvent::ClimbDown);
	}

	UpdateIndicator();
}

void UZCHUDLocalPlayerSubsystem::OnCanClimbDownCliff(const bool bDownCliff)
{
	bDownCliff ? ShowFaceButtonEvent(EZCFaceButtonEvent::ClimbDown) : HideFaceButtonEvent(EZCFaceButtonEvent::ClimbDown);
}

void UZCHUDLocalPlayerSubsystem::SetHUDModeState(EZCHUDMode NewMode)
{
	if (HUDState.HUDMode == NewMode) return;

	HUDState.HUDMode = NewMode;

	UpdateIndicator();
	UpdateFaceButton();
	UpdateCrosshair();
}

void UZCHUDLocalPlayerSubsystem::AddFaceButtonContext(EZCFaceButtonEvent AddButtonEvent, EFaceButtonType AddFaceButton, const FText& Label)
{
	FaceButtonMap.Emplace(AddButtonEvent, {AddFaceButton, Label});
}

void UZCHUDLocalPlayerSubsystem::AddSkillButtonContext(ESkillType SkillType, EFaceButtonType AddFaceButton, const FText& Label)
{
	SkillContextMap.FindOrAdd(SkillType).Emplace(AddFaceButton, Label);
}

void UZCHUDLocalPlayerSubsystem::ShowFaceButtonEvent(EZCFaceButtonEvent ShowButtonEvent)
{
	HUDState.Normal.FaceButtonEvent.Add(ShowButtonEvent);

	UpdateFaceButton();
}

void UZCHUDLocalPlayerSubsystem::HideFaceButtonEvent(EZCFaceButtonEvent HideButtonEvent)
{
	HUDState.Normal.FaceButtonEvent.Remove(HideButtonEvent);

	UpdateFaceButton();
}

void UZCHUDLocalPlayerSubsystem::ResetFaceButtonEvent()
{
	HUDState.Normal.FaceButtonEvent.Reset();

	UpdateFaceButton();
}

void UZCHUDLocalPlayerSubsystem::ShowSkillButtonEvent(ESkillType SkillType, EFaceButtonType ShowFaceButton)
{
	if (HUDState.Skill.SkillType != SkillType) return;

	HUDState.Skill.FaceButtonType.Add(ShowFaceButton);

	UpdateFaceButton();
}

void UZCHUDLocalPlayerSubsystem::HideSkillButtonEvent(ESkillType SkillType, EFaceButtonType HideFaceButton)
{
	if (HUDState.Skill.SkillType != SkillType) return;

	HUDState.Skill.FaceButtonType.Remove(HideFaceButton);

	UpdateFaceButton();
}

void UZCHUDLocalPlayerSubsystem::ResetSkillButtonEvent()
{
	HUDState.Skill.FaceButtonType.Reset();

	UpdateFaceButton();
}

void UZCHUDLocalPlayerSubsystem::ShowShortCutUI(const FVector2D& InputDirection)
{
	CurrentShortCutType = EItemType::None;
	int32 TargetSwitcherIndex = -1;

	if (InputDirection.Y > 0.0)
	{
		CurrentShortCutType = EItemType::Normal;
		TargetSwitcherIndex = ZCHUD::ShortCutType::Normal;
	}
	else if (InputDirection.X < 0.0)
	{
		CurrentShortCutType = EItemType::Shield;
		TargetSwitcherIndex = ZCHUD::ShortCutType::Shield;
	}
	else
	{
		CurrentShortCutType = EItemType::Weapon;
		TargetSwitcherIndex = ZCHUD::ShortCutType::Weapon;
	}

	InGameWidget->ShowShortCutUI(TargetSwitcherIndex, PlayerController->GetZCPlayerState()->GetInventoryComponent()->ShowShortcutInventory(CurrentShortCutType));
}

void UZCHUDLocalPlayerSubsystem::HideShortCutUI()
{
	InGameWidget->HideShortCutUI();
}

void UZCHUDLocalPlayerSubsystem::ScrollShortCutItemList(float ScrollValue)
{
	if (!InGameWidget) return;

	int32 Direction = 0;
	if (ScrollValue > 0.0f) Direction = 1;
	else if (ScrollValue < 0.0f) Direction = -1;

	InGameWidget->ScrollShortCutItemList(Direction);
}

bool UZCHUDLocalPlayerSubsystem::GetCurrentShortCutSelection(int32& OutIdx, EItemType& OutType) const
{
	if (InGameWidget)
	{
		OutIdx = InGameWidget->GetCurrentShortCutItemID();
		OutType = CurrentShortCutType;
		return true;
	}
	return false;
}

void UZCHUDLocalPlayerSubsystem::ShowSkillIndicator(ESkillType SkillType, int32 StateIdx)
{
	int32 NewMainIdx = GetMainIndicatorIndexFromSkill(SkillType);
	int32 NewSubIdx = GetSubIndicatorIndexFromSkillState(SkillType, StateIdx);

	const bool bIsMainChanged = (HUDState.SkillIndicator.Main != NewMainIdx);
	const bool bIsSubChanged = (HUDState.SkillIndicator.Sub != NewSubIdx);

	if (!bIsMainChanged && !bIsSubChanged) return;

	HUDState.SkillIndicator.Main = NewMainIdx;
	HUDState.SkillIndicator.Sub = NewSubIdx;

	UpdateIndicator();
}

void UZCHUDLocalPlayerSubsystem::ShowCrosshair()
{
	HUDState.CrosshairState = EZCCrosshairState::Normal;

	UpdateCrosshair();
}

void UZCHUDLocalPlayerSubsystem::HideCrosshair()
{
	HUDState.CrosshairState = EZCCrosshairState::None;

	UpdateCrosshair();
}

void UZCHUDLocalPlayerSubsystem::SetCrosshairLockOn(bool bLocked)
{
	if (HUDState.CrosshairState == EZCCrosshairState::None) return;

	EZCCrosshairState NewState = bLocked ? EZCCrosshairState::LockedOn : EZCCrosshairState::Normal;

	if (HUDState.CrosshairState == NewState) return;

	HUDState.CrosshairState = NewState;
	UpdateCrosshair();
}

int32 UZCHUDLocalPlayerSubsystem::GetMainIndicatorIndexFromSkill(ESkillType SkillType) const
{
	switch (SkillType)
	{
	case ESkillType::Ultrahand:
		return ZCHUD::ActionIndicator::Ultrahand;
	case ESkillType::Fuse:
		return ZCHUD::ActionIndicator::Fuse;
	case ESkillType::Ascend:
		return ZCHUD::ActionIndicator::Ascend;
	case ESkillType::Recall:
		return ZCHUD::ActionIndicator::Recall;
	case ESkillType::Autobuild:
		return ZCHUD::ActionIndicator::Autobuild;
	case ESkillType::Max:
		return INDEX_NONE;
	default:
		return INDEX_NONE;
	}
}

int32 UZCHUDLocalPlayerSubsystem::GetSubIndicatorIndexFromSkillState(ESkillType SkillType, int32 StateIndex) const
{
	switch (SkillType)
	{
	case ESkillType::Ultrahand:
		return StateIndex - 1;
	case ESkillType::Fuse:
	case ESkillType::Ascend:
	case ESkillType::Recall:
	case ESkillType::Autobuild:
	case ESkillType::Max:
	default:
		return StateIndex;
	}
}

int32 UZCHUDLocalPlayerSubsystem::GetFaceButtonModeIndexFromSkill(ESkillType SkillType) const
{
	switch (SkillType)
	{
	case ESkillType::Ultrahand: return ZCHUD::FaceButtonMode::Ultrahand;
	case ESkillType::Fuse:      return ZCHUD::FaceButtonMode::Fuse;
	case ESkillType::Ascend:    return ZCHUD::FaceButtonMode::Ascend;
	case ESkillType::Recall:    return ZCHUD::FaceButtonMode::Recall;
	case ESkillType::Autobuild: return ZCHUD::FaceButtonMode::Autobuild;
	default:                    return ZCHUD::FaceButtonMode::Normal;
	}
}

void UZCHUDLocalPlayerSubsystem::UpdateFaceButton()
{
	const EFaceButtonType AllButtons[] = { EFaceButtonType::AButton, EFaceButtonType::BButton, EFaceButtonType::XButton, EFaceButtonType::YButton };

	int32 TargetModeIndex = ZCHUD::FaceButtonMode::Normal;

	if (HUDState.HUDMode == EZCHUDMode::Skill)
	{
		TargetModeIndex = GetFaceButtonModeIndexFromSkill(HUDState.Skill.SkillType);
	}

	InGameWidget->SetFaceButtonMode(TargetModeIndex);

	switch (HUDState.HUDMode)
	{
	case EZCHUDMode::Normal:
	{
		const EZCFaceButtonEvent& CurrentEvent = HUDState.Normal.FaceButtonEvent.GetPriority();
		const FZCFaceButtonContext* FoundContext = FaceButtonMap.Find(CurrentEvent);

		// 표시할 대상이 없거나 이벤트가 None이면 전체 숨김
		if (CurrentEvent == EZCFaceButtonEvent::None || !FoundContext)
		{
			InGameWidget->HideFaceButtonUI();
		}
		else
		{
			// 모든 버튼을 순회하며 텍스트 설정
			for (const EFaceButtonType& Button : AllButtons)
			{
				if (Button == FoundContext->FaceButton)
				{
					// 현재 위젯(스위처에서 선택된)에 텍스트 업데이트
					InGameWidget->UpdateFaceButtonState(Button, FoundContext->Label);
				}
				else
				{
					InGameWidget->UpdateFaceButtonState(Button, FText());
				}
			}
		}
	}
	break;

	case EZCHUDMode::Skill:
	{
		ESkillType CurrentSkill = HUDState.Skill.SkillType;

		if (const TMap<EFaceButtonType, FText>* ButtonContexts = SkillContextMap.Find(CurrentSkill))
		{
			bool bAnyButtonShown = false;

			for (const EFaceButtonType& Button : AllButtons)
			{
				if (HUDState.Skill.FaceButtonType.HasFlag(Button))
				{
					if (const FText* Label = ButtonContexts->Find(Button))
					{
						InGameWidget->UpdateFaceButtonState(Button, *Label);
						bAnyButtonShown = true;
					}
				}
				else
				{
					// 플래그가 없는 버튼은 텍스트 비움
					InGameWidget->UpdateFaceButtonState(Button, FText());
				}
			}

			if (!bAnyButtonShown)
			{
				InGameWidget->HideFaceButtonUI();
			}
		}
		else
		{
			InGameWidget->HideFaceButtonUI();
		}
	}
	break;

	case EZCHUDMode::Gear:
		InGameWidget->HideFaceButtonUI();
		break;

	default:
		break;
	}
}

void UZCHUDLocalPlayerSubsystem::UpdateIndicator()
{
	//UZCLogger::Warning(TEXT("{0} : {1}"), HUDState.Normal.Movement, HUDState.Normal.CustomMovement);

	switch (HUDState.HUDMode)
	{
	case EZCHUDMode::Normal:
	{
		const EMovementMode CurrentMoveMode = HUDState.Normal.Movement;
		const ECustomMovementMode CurrentCustomMode = HUDState.Normal.CustomMovement;

		if (CurrentMoveMode != EMovementMode::MOVE_Custom)
		{
			switch (CurrentMoveMode)
			{
			case EMovementMode::MOVE_Falling:
				InGameWidget->ShowActionIndicatorUI(ZCHUD::ActionIndicator::Type::Falling);
				break;
			default:
				InGameWidget->HideActionIndicatorUI();
				break;
			}
		}
		else
		{
			switch (CurrentCustomMode)
			{
			case ECustomMovementMode::CMOVE_Climbing:
				InGameWidget->ShowActionIndicatorUI(ZCHUD::ActionIndicator::Type::Climb);
				break;
			case ECustomMovementMode::CMOVE_Gliding:
				InGameWidget->ShowActionIndicatorUI(ZCHUD::ActionIndicator::Type::Glide);
				break;
			case ECustomMovementMode::CMOVE_MAX:
				InGameWidget->HideActionIndicatorUI();
				break;
			default:
				break;
			}
		}
	}
		break;
	case EZCHUDMode::Skill:
	{
		if (HUDState.SkillIndicator.Main != INDEX_NONE && HUDState.SkillIndicator.Sub != INDEX_NONE)
		{
			InGameWidget->ShowActionIndicatorUI(HUDState.SkillIndicator.Main, HUDState.SkillIndicator.Sub);
		}
		else
		{
			InGameWidget->HideActionIndicatorUI();
		}
	}
		break;
	case EZCHUDMode::Gear: // 기어 상태 추가
		InGameWidget->HideActionIndicatorUI();
		break;
	default:
		break;
	}
}

void UZCHUDLocalPlayerSubsystem::UpdateCrosshair()
{
	if ((HUDState.HUDMode == EZCHUDMode::Skill) && (HUDState.CrosshairState != EZCCrosshairState::None))
	{
		InGameWidget->ShowCrosshairUI();

		const bool bIsLockOn = (HUDState.CrosshairState == EZCCrosshairState::LockedOn);

		InGameWidget->SetLockOnCrosshair(bIsLockOn);
	}
	else
	{
		HUDState.CrosshairState = EZCCrosshairState::None;
		InGameWidget->SetLockOnCrosshair(false);
		InGameWidget->HideCrosshairUI();
	}
}
