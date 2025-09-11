// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ItemListEntry/ZCItemListObject.h"
#include "Game/Subsystem/ZCItemGISubsystem.h"

namespace CustomFont
{
	namespace Weapon
	{
		FString OneHand = TEXT("C");
		FString TwoHand = TEXT("A");
		FString Spear	= TEXT("B");
	}
}

void UZCItemListObject::InitializeFromData(const FZCInventoryUIEntry& ItemKey, const uint8 ItemCount)
{
	const FZCItemTable& ItemData = *GetWorld()->GetGameInstance()->GetSubsystem<UZCItemGISubsystem>()->GetItemTable(ItemKey.Type, ItemKey.ID);
	
	// 인벤토리에서 데이터를 가져옴
	ID = ItemKey.ID;
	Durability = ItemKey.Durability;
	InventoryIndex = ItemKey.InventoryIndex;

	// 아이템 테이블에서 데이터를 가져옴
	Type = ItemData.Type;
	Rarity = ItemData.Rarity;
	Name = ItemData.Name;
	Description = ItemData.Description;
	Icon = ItemData.Icon;

	// 파라미터에서 가져옴
	Count = ItemCount;
	

	// 인벤토리 아이템의 내구도와 아이템 테이블의 내구도를 비교해 같으면 새 아이템으로 표시
	bIsNewItem = Durability == ItemData.Durability;

	Info_Icon = FString();

	// 무기의 경우 우측 하단에 무기 타입 아이콘 표시(커스텀 폰트로 구현되어 있음)
	switch (Type)
	{
		case EItemType::Weapon:
		{
			const FZCWeaponTable* WeaponData = static_cast<const FZCWeaponTable*>(&ItemData);
			Option = WeaponData->AttackPower;

			switch (WeaponData->WeaponType)
			{
			case EWeaponType::OneHandSword:
				Info_Icon = CustomFont::Weapon::OneHand;
				break;

			case EWeaponType::TwoHandSword:
				Info_Icon = CustomFont::Weapon::TwoHand;
				break;
			case EWeaponType::Spear:
				Info_Icon = CustomFont::Weapon::Spear;
				break;
			}
		}
	}
}
