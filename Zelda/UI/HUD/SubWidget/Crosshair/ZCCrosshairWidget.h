// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZCCrosshairWidget.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	void LockOn();
	void LockOff();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> CrosshairSwitcher;
};
