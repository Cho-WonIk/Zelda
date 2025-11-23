// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/SubWidget/ZCShortCutCommonWidget.h"
#include "CommonListView.h"
#include "Components/TextBlock.h"
#include "GameData/Table/ZCItemTable.h"
#include "UI/Common/ItemListEntry/ZCItemListObject.h"


void UZCShortCutCommonWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UZCShortCutCommonWidget::ScrollItemList(int32 Direction)
{
	const int32 ItemCount = ItemList->GetNumItems();
	if (ItemCount == 0) return;

	CurrentSelectedIndex = FMath::Clamp(CurrentSelectedIndex + Direction, 0, ItemCount - 1);

	FocusSelectedItem();
}

void UZCShortCutCommonWidget::SetItemList(const TArray<TPair<FZCInventoryUIEntry, uint8>>& ShortCutItems)
{
	ItemList->ClearListItems();

	// 0번 인덱스( 무기 없음)
	UZCItemListObject* EmptyObject = NewObject<UZCItemListObject>(this);
	if (EmptyObject)
	{
		EmptyObject->EmptyObject();
		EmptyObject->Name = TEXT("없음");
		ItemList->AddItem(EmptyObject);
	}

	for (const auto &Item : ShortCutItems)
	{
		UZCItemListObject* ItemObject = NewObject<UZCItemListObject>(this);
		if (ItemObject)
		{
			ItemObject->InitializeFromData(Item.Key, Item.Value);
			ItemList->AddItem(ItemObject);
		}
	}

	FocusSelectedItem();
}

int32 UZCShortCutCommonWidget::GetCurrentSelectedItemID() const
{
	// 0번 인덱스 : 장착 해제
	if (CurrentSelectedIndex == 0) return -1;

	if (UZCItemListObject* ZCItem = Cast<UZCItemListObject>(ItemList->GetItemAt(CurrentSelectedIndex)))
	{
		// 일반 아이템의 경우 ID값, 무기, 방패의 경우 인벤토리 인덱스값을 반환
		return ZCItem->InventoryIndex;
	}

	return -1;
}

void UZCShortCutCommonWidget::FocusSelectedItem()
{
	if (!ItemList->GetItemAt(CurrentSelectedIndex))
	{
		CurrentSelectedIndex = 0;
	}

	if (UZCItemListObject* ZCItem = Cast<UZCItemListObject>(ItemList->GetItemAt(CurrentSelectedIndex)))
	{
		if (ZCItem->ID == -1)
		{
			ItemNameText->SetText(FText::FromString(TEXT("해제")));
			ItemDescriptionText->SetText(FText::FromString(TEXT("장비를 해제합니다")));
		}

		ItemNameText->SetText(FText::FromString(ZCItem->Name));
		ItemDescriptionText->SetText(FText::FromString(ZCItem->Description));

		ItemList->SetSelectedItem(ZCItem);
		ItemList->ScrollIndexIntoView(CurrentSelectedIndex);
	}
}

