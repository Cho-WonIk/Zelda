// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/FaceButton/ZCFaceButtonWidget.h"
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
	AButton_Text->SetVisibility(ESlateVisibility::Hidden);
	BButton_Text->SetVisibility(ESlateVisibility::Hidden);
	XButton_Text->SetVisibility(ESlateVisibility::Hidden);
	YButton_Text->SetVisibility(ESlateVisibility::Hidden);

    switch (ButtonType)
    {
    case EFaceButtonType::AButton:
        if (AButton_Text)
        {
            AButton_Text->SetText(Text);
            AButton_Text->SetVisibility(ESlateVisibility::Visible);
        }
        break;
    case EFaceButtonType::BButton:
        if (BButton_Text)
        {
            BButton_Text->SetText(Text);
            BButton_Text->SetVisibility(ESlateVisibility::Visible);
        }
        break;
    case EFaceButtonType::XButton:
        if (XButton_Text)
        {
            XButton_Text->SetText(Text);
            XButton_Text->SetVisibility(ESlateVisibility::Visible);
        }
        break;
    case EFaceButtonType::YButton:
        if (YButton_Text)
        {
            YButton_Text->SetText(Text);
            YButton_Text->SetVisibility(ESlateVisibility::Visible);
        }
        break;
    default:
        break;
    }
}
