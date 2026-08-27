// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/State/ZCBarWidget.h"
#include "Components/ProgressBar.h"
#include "Character/ZCCharacter.h"

void UZCBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

}

void UZCBarWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UZCBarWidget::SetBarPercent_Implementation(float Percent)
{
	Bar->SetPercent(Percent);
}
