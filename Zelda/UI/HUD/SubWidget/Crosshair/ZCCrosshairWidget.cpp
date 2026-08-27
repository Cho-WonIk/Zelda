// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/SubWidget/Crosshair/ZCCrosshairWidget.h"
#include "Components/WidgetSwitcher.h"

void UZCCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UZCCrosshairWidget::LockOn()
{
	CrosshairSwitcher->SetActiveWidgetIndex(1);
}

void UZCCrosshairWidget::LockOff()
{
	CrosshairSwitcher->SetActiveWidgetIndex(0);
}
