// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/SubWidget/Indicator/ZCIndicatorWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/Spacer.h"
#include "UI/Common/ActionKey/ZCActionKeyWidget.h"
#include "CommonActionWidget.h"
#include "CommonTextBlock.h"

void UZCIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Init();
}

void UZCIndicatorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		Init();
	}
}

void UZCIndicatorWidget::Init()
{
	for (const auto& Indicator : IndicatorArray)
	{
		if (!Indicator.Class) continue;

		UZCActionKeyWidget* NewWidget = CreateWidget<UZCActionKeyWidget>(this, Indicator.Class);
		NewWidget->SetActionIndicator(Indicator.Text);
		HorizonBox->AddChildToHorizontalBox(NewWidget);

		if (&Indicator != &IndicatorArray.Last())
		{
			USpacer* Spacer = NewObject<USpacer>(this, USpacer::StaticClass());
			Spacer->SetSize(SpaceOffset);
			HorizonBox->AddChildToHorizontalBox(Spacer);
		}
	}
}