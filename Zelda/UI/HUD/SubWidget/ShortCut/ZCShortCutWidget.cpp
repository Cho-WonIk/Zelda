// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/SubWidget/ShortCut/ZCShortCutWidget.h"
#include "CommonListView.h"
#include "Components/TextBlock.h"
#include "GameData/Table/ZCItemTable.h"
#include "UI/Common/ItemListEntry/ZCShortcutListObject.h"

void UZCShortCutWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UZCShortCutWidget::ScrollItemList(int32 Direction)
{
	const int32 ItemCount = ItemList->GetNumItems();
	if (ItemCount == 0) return;

	CurrentSelectedIndex = FMath::Clamp(CurrentSelectedIndex + Direction, 0, ItemCount - 1);

	FocusSelectedItem();
}

void UZCShortCutWidget::SetItemList(const TArray<UZCShortcutListObject*>& ShortCutItems)
{
	ItemList->ClearListItems();

	// 0번 인덱스( 무기 없음)
	UZCShortcutListObject* EmptyObject = NewObject<UZCShortcutListObject>(this);
	if (EmptyObject)
	{
		EmptyObject->Name = FText::FromString(TEXT("없음"));
		ItemList->AddItem(EmptyObject);
	}

	for (const auto& Item : ShortCutItems)
	{
		if (Item)
		{
			ItemList->AddItem(Item);
		}
	}

	FocusSelectedItem();
}

int32 UZCShortCutWidget::GetCurrentSelectedItemID() const
{
	// 0번 인덱스 : 장착 해제
	if (CurrentSelectedIndex == 0) return -1;

	if (UZCShortcutListObject* ZCItem = Cast<UZCShortcutListObject>(ItemList->GetItemAt(CurrentSelectedIndex)))
	{
		// 일반 아이템의 경우 ID값, 무기, 방패의 경우 인벤토리 인덱스값을 반환
		return ZCItem->InventoryIndex;
	}

	return -1;
}

void UZCShortCutWidget::FocusSelectedItem()
{
	if (!ItemList->GetItemAt(CurrentSelectedIndex))
	{
		CurrentSelectedIndex = 0;
	}

	if (UZCShortcutListObject* ZCItem = Cast<UZCShortcutListObject>(ItemList->GetItemAt(CurrentSelectedIndex)))
	{
		ItemName->SetText(ZCItem->Name);
		ItemDescription->SetText(ZCItem->Description);

		ItemList->SetSelectedItem(ZCItem);
		ItemList->ScrollIndexIntoView(CurrentSelectedIndex);
	}
}