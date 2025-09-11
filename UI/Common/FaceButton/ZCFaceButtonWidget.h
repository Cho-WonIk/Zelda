// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZCFaceButtonWidget.generated.h"

class UTextBlock;
class UCommonUserWidget;

UENUM(BlueprintType)
enum EFaceButtonType : uint8
{
	AButton,
	BButton,
	XButton,
	YButton
};

UCLASS()
class ZELDA_API UZCFaceButtonWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
public:
	// 버튼 텍스트 설정
	void SetButtonText(EFaceButtonType ButtonType, const FText& Text);

protected:
	// Face Button UI, Xboxt 컨트롤러 기준
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AButton_Text;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BButton_Text;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> XButton_Text;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> YButton_Text;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCommonUserWidget> WBP_A_Button;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCommonUserWidget> WBP_B_Button;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCommonUserWidget> WBP_X_Button;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCommonUserWidget> WBP_Y_Button;

};
