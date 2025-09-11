// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZCBarWidget.generated.h"

class UProgressBar;
class AZCCharacter;

UCLASS()
class ZELDA_API UZCBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintNativeEvent, Category = "UI")
	void SetBarPercent(float Percent);

	virtual void SetBarPercent_Implementation(float Percent);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (Bindwidget))
	TObjectPtr<UProgressBar> Bar;
};
