// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/ZCHUDCommonWidget.h"
#include "Components/WidgetSwitcher.h"

#include "Component/Inventory/ZCInventoryComponent.h"
#include "Player/ZCPlayerController.h"
#include "Player/ZCPlayerState.h"

#include "GameData/ZCItemTable.h"

#include "UI/Common/FaceButton/ZCFaceButtonWidget.h"
#include "UI/HUD/SubWidget/ZCShortCutCommonWidget.h"


void UZCHUDCommonWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayerController = Cast<AZCPlayerController>(GetOwningPlayer());
	check(PlayerController);

	PlayerState = PlayerController->GetZCPlayerStateChecked<AZCPlayerState>();

	HideActionIndicatorUI();
	HideFaceButtonUI();
	HideShortCutUI();
}

void UZCHUDCommonWidget::ShowActionIndicatorUI(int32 IndicatorIdx)
{
	ActionIndicatorSwitcher->SetVisibility(ESlateVisibility::Visible);
	ActionIndicatorSwitcher->SetActiveWidgetIndex(IndicatorIdx);
}

void UZCHUDCommonWidget::HideActionIndicatorUI()
{
	ActionIndicatorSwitcher->SetVisibility(ESlateVisibility::Hidden);
}

void UZCHUDCommonWidget::ShowFaceButtonUI(const EFaceButtonType CurrentButton, const FText& Text)
{
	FaceButtonWidget->SetVisibility(ESlateVisibility::Visible);
	FaceButtonWidget->SetButtonText(CurrentButton, Text);
}

void UZCHUDCommonWidget::HideFaceButtonUI()
{
	FaceButtonWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UZCHUDCommonWidget::ShowShortCutUI(EItemType ItemType)
{
	ShortCutSwitcher->SetVisibility(ESlateVisibility::Visible);

	switch (ItemType)
	{
	case EItemType::Normal:
		ShortCutSwitcher->SetActiveWidgetIndex(Widget::ShortCutType::Normal);
		break;
	case EItemType::Weapon:
		ShortCutSwitcher->SetActiveWidgetIndex(Widget::ShortCutType::Weapon);
		break;
	case EItemType::Shield:
		ShortCutSwitcher->SetActiveWidgetIndex(Widget::ShortCutType::Shield);
		break;

	default:
		return;
	}

	UWidget* ActiveWidget = ShortCutSwitcher->GetActiveWidget();

	CurrentShortCutWidget = Cast<UZCShortCutCommonWidget>(ActiveWidget);

	CurrentShortCutWidget->SetItemList(PlayerState->GetInventoryComponent()->ShowInventory(ItemType));
}

void UZCHUDCommonWidget::ScrollShortCutItemList(int32 Direction)
{
	CurrentShortCutWidget->ScrollItemList(Direction);
}

void UZCHUDCommonWidget::HideShortCutUI()
{
	ShortCutSwitcher->SetVisibility(ESlateVisibility::Hidden);
}

int32 UZCHUDCommonWidget::GetCurrentShortCutItemID() const
{
	return CurrentShortCutWidget ? CurrentShortCutWidget->GetCurrentSelectedItemID() : -1;
}
