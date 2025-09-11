// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ZCShortCutCommonWidget.generated.h"

struct FZCInventoryUIEntry;

UCLASS()
class ZELDA_API UZCShortCutCommonWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	void ScrollItemList(int32 Direction);

	void SetItemList(const TArray<TPair<FZCInventoryUIEntry, uint8>>& ShortCutItems);

	// 숏컷 UI에서 선택된 아이템의 정보를 반환(무기, 방패는 Index값, 일반 아이템은 ID값)
	int32 GetCurrentSelectedItemID() const;

protected:
	void FocusSelectedItem();

protected:
	int32 CurrentSelectedIndex = 0;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCommonListView> ItemList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ItemDescriptionText;
};
