// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZCIndicatorWidget.generated.h"

class UCommonActionWidget;
class UCommonTextBlock;
class UZCActionKeyWidget;

USTRUCT(BlueprintType)
struct FZCIndicatorStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UZCActionKeyWidget> Class;

	UPROPERTY(EditAnywhere)
	FText Text;
};

UCLASS()
class ZELDA_API UZCIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	void Init();

public:
	UPROPERTY(EditAnywhere)
	TArray<struct FZCIndicatorStruct> IndicatorArray;

	UPROPERTY(EditAnywhere)
	FVector2D SpaceOffset;

protected:
	UPROPERTY(meta = (Bindwidget))
	TObjectPtr<class UHorizontalBox> HorizonBox;
};