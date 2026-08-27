// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/SubWidget/FaceButton/ZCFaceButtonWidget.h"
#include "Components/TextBlock.h"
#include "CommonUserWidget.h"

void UZCFaceButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AButton_Text->SetVisibility(ESlateVisibility::Hidden);
	BButton_Text->SetVisibility(ESlateVisibility::Hidden);
	XButton_Text->SetVisibility(ESlateVisibility::Hidden);
	YButton_Text->SetVisibility(ESlateVisibility::Hidden);
}

void UZCFaceButtonWidget::SetButtonText(EFaceButtonType ButtonType, const FText& Text)
{
	auto UpdateButtonVisibility = [](UTextBlock* InTextBlock, const FText& InText)
		{
			if (!InTextBlock) return;

			// 텍스트가 비어있으면 숨김 (Hidden), 있으면 보임 (Visible/HitTestInvisible)
			if (InText.IsEmpty())
			{
				InTextBlock->SetVisibility(ESlateVisibility::Hidden);
			}
			else
			{
				InTextBlock->SetText(InText);
				InTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		};
	switch (ButtonType)
	{
	case EFaceButtonType::AButton:
		UpdateButtonVisibility(AButton_Text, Text);
		break;
	case EFaceButtonType::BButton:
		UpdateButtonVisibility(BButton_Text, Text);
		break;
	case EFaceButtonType::XButton:
		UpdateButtonVisibility(XButton_Text, Text);
		break;
	case EFaceButtonType::YButton:
		UpdateButtonVisibility(YButton_Text, Text);
		break;
	default:
		break;
	}
}

void UZCFaceButtonWidget::ResetButtons()
{
	if (AButton_Text) AButton_Text->SetVisibility(ESlateVisibility::Hidden);
	if (BButton_Text) BButton_Text->SetVisibility(ESlateVisibility::Hidden);
	if (XButton_Text) XButton_Text->SetVisibility(ESlateVisibility::Hidden);
	if (YButton_Text) YButton_Text->SetVisibility(ESlateVisibility::Hidden);
}
