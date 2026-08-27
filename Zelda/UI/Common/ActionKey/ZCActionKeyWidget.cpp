// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ActionKey/ZCActionKeyWidget.h"
#include "CommonActionWidget.h"
#include "CommonTextBlock.h"

void UZCActionKeyWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UZCActionKeyWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UZCActionKeyWidget::SetActionIndicator(const FText& NewActionText)
{
	if (!ActionText) return;
	ActionText->SetText(NewActionText);
}
