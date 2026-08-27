// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ZCItemEntryWidget.generated.h"

UCLASS()
class ZELDA_API UZCItemEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	// 아이템 객체 세팅될 때 (IUserObjectListEntry)
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	// 선택 상태 변경 콜백 (IUserListEntry)
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Material;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Info;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ShowNewItem;

protected:
	UPROPERTY()
	UMaterialInstanceDynamic* MID = nullptr;
};
