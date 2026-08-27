// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/ZCHUDWidget.h"
#include "Components/WidgetSwitcher.h"

#include "Component/Inventory/ZCInventoryComponent.h"

#include "GameData/Table/ZCItemTable.h"

#include "UI/HUD/SubWidget/FaceButton/ZCFaceButtonWidget.h"
#include "UI/HUD/SubWidget/ShortCut/ZCShortCutWidget.h"
#include "UI/HUD/SubWidget/Crosshair/ZCCrosshairWidget.h"

#include "Development/ZCLogger.h"

void UZCHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

///////////////////////////////////////////////////////////

void UZCInGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideActionIndicatorUI();
	HideFaceButtonUI();
	HideShortCutUI();
	HideCrosshairUI();
}

void UZCInGameHUDWidget::ShowActionIndicatorUI(int32 MainIdx, int32 SubIdx)
{
	ActionIndicatorSwitcher->SetVisibility(ESlateVisibility::Visible);
	ActionIndicatorSwitcher->SetActiveWidgetIndex(MainIdx);

	if (SubIdx != INDEX_NONE)
	{
		UWidget* ActiveWidget = ActionIndicatorSwitcher->GetActiveWidget();
		if (UWidgetSwitcher* SubSwitcher = Cast<UWidgetSwitcher>(ActiveWidget))
		{
			// 자식 스위처의 인덱스 유효성 검사
			if (SubIdx >= 0 && SubIdx < SubSwitcher->GetNumWidgets())
			{
				SubSwitcher->SetActiveWidgetIndex(SubIdx);
			}

		}

	}
}

void UZCInGameHUDWidget::HideActionIndicatorUI()
{
	ActionIndicatorSwitcher->SetVisibility(ESlateVisibility::Hidden);
}

void UZCInGameHUDWidget::SetFaceButtonMode(int32 ModeIndex)
{
	FaceButtonSwitcher->SetVisibility(ESlateVisibility::Visible);
	FaceButtonSwitcher->SetActiveWidgetIndex(ModeIndex);

	CurrentFaceButtonWidget = Cast<UZCFaceButtonWidget>(FaceButtonSwitcher->GetActiveWidget());
}

void UZCInGameHUDWidget::UpdateFaceButtonState(const EFaceButtonType CurrentButton, const FText& Text)
{
	if (CurrentFaceButtonWidget)
	{
		CurrentFaceButtonWidget->SetVisibility(ESlateVisibility::Visible);
		CurrentFaceButtonWidget->SetButtonText(CurrentButton, Text);
	}
}

void UZCInGameHUDWidget::HideFaceButtonUI()
{
	FaceButtonSwitcher->SetVisibility(ESlateVisibility::Hidden);
}

void UZCInGameHUDWidget::ShowShortCutUI(int32 SwitcherIndex, const TArray<class UZCShortcutListObject*>& ItemList)
{
	ShortCutSwitcher->SetVisibility(ESlateVisibility::Visible);

	UWidget* ActiveWidget = ShortCutSwitcher->GetActiveWidget();
	CurrentShortCutWidget = Cast< UZCShortCutWidget>(ActiveWidget);
	CurrentShortCutWidget->SetItemList(ItemList);
}

void UZCInGameHUDWidget::ScrollShortCutItemList(int32 Direction)
{
	//if (!CurrentShortCutWidget) return;
	CurrentShortCutWidget->ScrollItemList(Direction);
}

void UZCInGameHUDWidget::HideShortCutUI()
{
	ShortCutSwitcher->SetVisibility(ESlateVisibility::Hidden);
}

int32 UZCInGameHUDWidget::GetCurrentShortCutItemID() const
{
	return CurrentShortCutWidget ? CurrentShortCutWidget->GetCurrentSelectedItemID() : -1;
}

void UZCInGameHUDWidget::ShowCrosshairUI()
{
	CrosshairWidget->SetVisibility(ESlateVisibility::Visible);
}

void UZCInGameHUDWidget::HideCrosshairUI()
{
	CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UZCInGameHUDWidget::SetLockOnCrosshair(bool bIsLockOn)
{
	if (CrosshairWidget)
	{
		if (bIsLockOn)
		{
			CrosshairWidget->LockOn();
		}
		else
		{
			CrosshairWidget->LockOff();
		}
	}
}
