// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ItemListEntry/ZCItemEntryCommonButton.h"
#include "Engine/StreamableManager.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "UI/Common/ItemListEntry/ZCItemListObject.h"
#include "Engine/AssetManager.h"

void UZCItemEntryCommonButton::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	SetIsFocusable(true);

	if (UZCItemListObject* EntryData = Cast<UZCItemListObject>(ListItemObject))
	{
		if (EntryData->ID == -1)
		{
			Is_NewItem->SetVisibility(ESlateVisibility::Hidden);
			Info_Border->SetVisibility(ESlateVisibility::Hidden);
			Info_Number->SetVisibility(ESlateVisibility::Hidden);
			Info_Icon->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			Info_Icon->SetText(FText::FromString(EntryData->Info_Icon));

			Border->SetBrushTintColor(FLinearColor::Black);

			Is_NewItem->SetVisibility(EntryData->bIsNewItem ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

			switch (EntryData->Type)
			{
			case EItemType::Weapon:
				Info_Number->SetText(FText::AsNumber(EntryData->Option));
				Info_Icon->SetText(FText::FromString(EntryData->Info_Icon));
				break;

			case EItemType::Normal:
				Info_Number->SetText(FText::AsNumber(EntryData->Count));
				break;

			default:
				break;
			}

			// 아이콘 로드
			if (EntryData->Icon.IsValid())
			{
				UTexture2D* LoadedTexture = EntryData->Icon.Get();
				if (LoadedTexture)
				{
					Icon->SetBrushFromTexture(LoadedTexture);
				}
			}
			else
			{
				// 비동기 로드
				FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
				Streamable.RequestAsyncLoad(EntryData->Icon.ToSoftObjectPath(), FStreamableDelegate::CreateLambda(
					[=, this]() { if (Icon) { Icon->SetBrushFromTexture(EntryData->Icon.Get()); } }
				));
			}
		}
	}
}

void UZCItemEntryCommonButton::NativeOnSelected(bool bBroadcast)
{
	Super::NativeOnSelected(bBroadcast);

	//UE_LOG(LogTemp, Warning, TEXT("아이템 선택됨"));
	Border->SetBrushTintColor(FLinearColor::Blue);
}

void UZCItemEntryCommonButton::NativeOnDeselected(bool bBroadcast)
{
	Super::NativeOnDeselected(bBroadcast);

	//UE_LOG(LogTemp, Warning, TEXT("아이템 선택 해제됨"));
	Border->SetBrushTintColor(FLinearColor::Black);
}
