// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZCShortCutWidget.generated.h"

class UZCShortcutListObject;

UCLASS()
class ZELDA_API UZCShortCutWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void ScrollItemList(int32 Direction);
	void SetItemList(const TArray<UZCShortcutListObject*>& ShortCutItems);

	// 숏컷 UI에서 선택된 아이템의 정보를 반환(무기, 방패는 Index값, 일반 아이템은 ID값)
	int32 GetCurrentSelectedItemID() const;

protected:
	void FocusSelectedItem();

protected:
	int32 CurrentSelectedIndex = 0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCommonListView> ItemList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemDescription;
};
