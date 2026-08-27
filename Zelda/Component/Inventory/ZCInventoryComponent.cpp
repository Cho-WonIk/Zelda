// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/ZCInventoryComponent.h"

#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Game/Subsystem/ZCItemGISubsystem.h"

#include "Actor/Item/Weapon/ZCWeaponActor.h"
#include "Actor/Item/Shield/ZCShieldActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCInventoryComponent)

// Sets default values for this component's properties
UZCInventoryComponent::UZCInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

// Called when the game starts
void UZCInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ItemGISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UZCItemGISubsystem>();
}

TArray<UZCShortcutListObject*> UZCInventoryComponent::ShowShortcutInventory(const EItemType Type)
{
	TArray<UZCShortcutListObject*> Result;

	switch (Type)
	{
	case EItemType::Normal:
	{
		for (const auto& Item : NormalInventory)
		{
			const FZCItemTable& ItemData = *ItemGISubsystem->GetNormalItemTable(Item.Key);

			UZCShortcutListObject* Entry = NewObject<UZCShortcutListObject>(this);
			Entry->ID = Item.Key;
			Entry->Type = Type;

			Entry->Rarity = ItemData.Rarity;
			Entry->Count = Item.Value;

			Entry->Name = ItemData.Name;
			Entry->Description = ItemData.Description;
			Entry->Icon = ItemData.Icon;

			Result.Emplace(Entry);
		}
		break;
	}
	case EItemType::Weapon:
	{
		int32 Idx = 0;
		for (const auto& Item : WeaponInventory)
		{
			const FZCWeaponTable* ItemData = Item.Get();
			if (ItemData == nullptr)
			{
				++Idx;
				continue;
			}

			UZCShortcutListObject* Entry = NewObject<UZCShortcutListObject>(this);
			Entry->ID = ItemData->ID;
			Entry->Type = Type;

			Entry->Rarity = ItemData->Rarity;
			Entry->Count = 1;

			Entry->Name = ItemData->Name;
			Entry->Description = ItemData->Description;

			Entry->Option = ItemData->AttackPower;

			Entry->InventoryIndex = Idx++;

			const FZCWeaponTable* OriginData = ItemGISubsystem->GetWeaponTable(ItemData->ID);
			Entry->bIsNewItem = (OriginData != nullptr) && (ItemData->Durability == OriginData->Durability);

			Entry->Icon = ItemData->Icon;

			switch (ItemData->WeaponType)
			{
			case EWeaponType::OneHandSword:
				Entry->Info = CustomFont::Weapon::OneHand;
				break;
			case EWeaponType::TwoHandSword:
				Entry->Info = CustomFont::Weapon::TwoHand;
				break;
			case EWeaponType::Spear:
				Entry->Info = CustomFont::Weapon::Spear;
				break;
			default:
				Entry->Info.Empty();
				break;
			}

			Result.Emplace(Entry);
		}
		break;
	}
	case EItemType::Shield:
	{
		int32 Idx = 0;
		for (const auto& Item : ShieldInventory)
		{
			const FZCShieldTable* ItemData = Item.Get();
			if (ItemData == nullptr)
			{
				++Idx;
				continue;
			}

			UZCShortcutListObject* Entry = NewObject<UZCShortcutListObject>(this);
			Entry->ID = ItemData->ID;
			Entry->Type = Type;

			Entry->Rarity = ItemData->Rarity;
			Entry->Count = 1;

			Entry->Name = ItemData->Name;
			Entry->Description = ItemData->Description;

			// Entry->Option = ItemData->DefensePower;

			Entry->InventoryIndex = Idx++;

			const FZCShieldTable* OriginData = ItemGISubsystem->GetShieldTable(ItemData->ID);
			Entry->bIsNewItem = (OriginData != nullptr) && (ItemData->Durability == OriginData->Durability);

			Entry->Icon = ItemData->Icon;

			Result.Emplace(Entry);
		}
		break;
	}
	default:
		break;
	}

	return Result;
}

bool UZCInventoryComponent::AddItem(TUniquePtr<struct FZCItemTable> NewItem)
{
	if (!NewItem.IsValid()) return false;

	switch (NewItem->ItemType)
	{
	case EItemType::Normal:
		return AddNormalItem(MoveTemp(NewItem));
	case EItemType::Weapon:
		return AddWeaponItem(TUniquePtr<FZCWeaponTable>(static_cast<FZCWeaponTable*>(NewItem.Release())));
	case EItemType::Shield:
		return AddShieldItem(TUniquePtr<FZCShieldTable>(static_cast<FZCShieldTable*>(NewItem.Release())));
	default:
		return false;
	}

	return false;
}

void UZCInventoryComponent::DropItem(const EItemType Type, const int32 Index)
{
	UZCWorldSubsystem* WorldSubsytem = GetWorld()->GetSubsystem<UZCWorldSubsystem>();
	if (!WorldSubsytem) return;

	switch (Type)
	{
	case EItemType::Normal:
	{
		
		break;
	}
	case EItemType::Weapon:
	{
		
		break;
	}
	case EItemType::Shield:
	{
		
		break;
	}
	default:
		break;
	}
}

void UZCInventoryComponent::RemoveItem(const EItemType Type, const FZCItemTable& RemoveItem, const int32 Index)
{
	switch (Type)
	{
	case EItemType::Normal:
	{
		if (uint8* Count = NormalInventory.Find(RemoveItem.ID))
		{
			if (--(*Count) <= 0)
			{
				NormalInventory.Remove(RemoveItem.ID);
			}
		}
		break;
	}
	case EItemType::Weapon:
	{
		if (WeaponInventory.IsValidIndex(Index))
		{
			WeaponInventory.RemoveAt(Index, 1, EAllowShrinking::No);
		}
		break;
	}
	case EItemType::Shield:
	{
		if (ShieldInventory.IsValidIndex(Index))
		{
			ShieldInventory.RemoveAt(Index, 1, EAllowShrinking::No);
		}
		break;
	}
	default:
		break;
	}
}

AZCItemActor* UZCInventoryComponent::SpawnNormalItem(const int32 Index, const FTransform& Transform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride)
{
	return GetWorld()->GetSubsystem<UZCWorldSubsystem>()->SpawnItemByID<AZCItemActor>(EItemType::Normal, Index, Transform, nullptr, nullptr, CollisionHandlingOverride);
}

AZCWeaponActor* UZCInventoryComponent::SpawnWeaponItem(const int32 Index, const FTransform& Transform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride)
{
	return GetWorld()->GetSubsystem<UZCWorldSubsystem>()->SpawnItemByData<AZCWeaponActor>(WeaponInventory[Index].Get(), Index, Transform, nullptr, nullptr, CollisionHandlingOverride);
}

AZCShieldActor* UZCInventoryComponent::SpawnShieldItem(const int32 Index, const FTransform& Transform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride)
{
	return GetWorld()->GetSubsystem<UZCWorldSubsystem>()->SpawnItemByData<AZCShieldActor>(ShieldInventory[Index].Get(), Index, Transform, nullptr, nullptr, CollisionHandlingOverride);
}

bool UZCInventoryComponent::AddWeaponItem(TUniquePtr<struct FZCWeaponTable> NewWeapon)
{
	if (!NewWeapon.IsValid()) return false;

	for (int32 i = 0; i < WeaponInventory.Num(); i++)
	{
		if (!WeaponInventory[i].IsValid())
		{
			WeaponInventory[i] = MoveTemp(NewWeapon);
			return true;
		}
	}
	WeaponInventory.Add(MoveTemp(NewWeapon));
	return true;
}

bool UZCInventoryComponent::AddShieldItem(TUniquePtr<struct FZCShieldTable> NewShield)
{
	if (!NewShield.IsValid()) return false;
	for (int32 i = 0; i < ShieldInventory.Num(); i++)
	{
		if (!ShieldInventory[i].IsValid())
		{
			ShieldInventory[i] = MoveTemp(NewShield);
			return true;
		}
	}
	ShieldInventory.Add(MoveTemp(NewShield));
	return true;
}

bool UZCInventoryComponent::AddNormalItem(TUniquePtr<struct FZCItemTable> NewItem)
{
	if (!NewItem.IsValid()) return false;

	NormalInventory.FindOrAdd(NewItem->ID, 0)++;

	return true;
}
