// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ZCItemEntryCommonButton.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCItemEntryCommonButton : public UCommonButtonBase, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override; 

protected:
	virtual void NativeOnSelected(bool bBroadcast) override;
	virtual void NativeOnDeselected(bool bBroadcast) override;

protected:
	// 아이템 아이콘
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UImage> Icon;

	UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UImage> Border;

    // 아이템 옵션 혹은 아이템 갯수 표시란
    UPROPERTY(meta = (Bindwidget))
	TObjectPtr<class UBorder> Info_Border;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Info_Number;
	
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Info_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Is_NewItem;
    
};
