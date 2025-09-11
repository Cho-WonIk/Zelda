// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/Indicator/ZCActionIndicatorCommonWidget.h"
#include "CommonActionWidget.h"
#include "CommonTextBlock.h"

void UZCActionIndicatorCommonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetActionIndicator(ActionTextValue);
}

void UZCActionIndicatorCommonWidget::NativePreConstruct()
{
	SetActionIndicator(ActionTextValue);
}

void UZCActionIndicatorCommonWidget::SetActionIndicator(const FText& NewActionText)
{
	if (!ActionText) return;
	ActionTextValue = NewActionText;
	ActionText->SetText(NewActionText);
}
