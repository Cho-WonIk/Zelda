// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZCActionKeyWidget.generated.h"

class UCommonActionWidget;
class UCommonTextBlock;

UCLASS()
class ZELDA_API UZCActionKeyWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

public:
	void SetActionIndicator(const FText& NewActionText);

protected:
	// 이행가능한 동작 표시, Icon은 블루프린트에서 설정
	UPROPERTY(meta = (Bindwidget))
	TObjectPtr<class UCommonActionWidget> ActionWidget;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCommonTextBlock> ActionText;
};
