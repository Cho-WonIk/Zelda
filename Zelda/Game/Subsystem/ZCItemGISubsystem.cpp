// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Subsystem/ZCItemGISubsystem.h"
#include "Game/ZCGameInstance.h"

UZCItemGISubsystem& UZCItemGISubsystem::Get(UObject* Context)
{
	check(Context)
	return *Context->GetWorld()->GetGameInstance()->GetSubsystem<UZCItemGISubsystem>();
}

void UZCItemGISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UZCGameInstance* ZCGameInstance = Cast<UZCGameInstance>(GetGameInstance());

	const UDataTable* const NoramlItemTable = ZCGameInstance->GetNoramlItemTable();
	if (NoramlItemTable)
	{
		TArray<uint8*> RowData;
		NoramlItemTable->GetRowMap().GenerateValueArray(RowData);

		NormalItems.Reserve(RowData.Num());

		for (uint8* Row : RowData)
		{
			const FZCItemTable& Item = *reinterpret_cast<FZCItemTable*>(Row);
			int32 idx = NormalItems.Add(Item);
			NormalItemIndexMap.Add(Item.ID, idx);
		}
	}

	const UDataTable* const WeaponItemTable = ZCGameInstance->GetWeaponItemTable();
	if (WeaponItemTable)
	{
		TArray<uint8*> RowData;
		WeaponItemTable->GetRowMap().GenerateValueArray(RowData);
		WeaponItems.Reserve(RowData.Num());
		for (uint8* Row : RowData)
		{
			const FZCWeaponTable& Item = *reinterpret_cast<FZCWeaponTable*>(Row);
			int32 idx = WeaponItems.Add(Item);
			WeaponItemIndexMap.Add(Item.ID, idx);
		}
	}

	const UDataTable* const ShieldItemTable = ZCGameInstance->GetShieldItemTable();
	if (ShieldItemTable)
	{
		TArray<uint8*> RowData;
		ShieldItemTable->GetRowMap().GenerateValueArray(RowData);
		ShieldItems.Reserve(RowData.Num());
		for (uint8* Row : RowData)
		{
			const FZCShieldTable& Item = *reinterpret_cast<FZCShieldTable*>(Row);
			int32 idx = ShieldItems.Add(Item);
			ShieldItemIndexMap.Add(Item.ID, idx);
		}
	}
}

void UZCItemGISubsystem::Deinitialize()
{
	Super::Deinitialize();
	NormalItems.Empty();
	NormalItemIndexMap.Empty();

	WeaponItems.Empty();
	WeaponItemIndexMap.Empty();

	ShieldItems.Empty();
	ShieldItemIndexMap.Empty();
}

const FZCItemTable* UZCItemGISubsystem::GetItemTable(const EItemType Type, const int32 ID) const
{
	switch (Type)
	{
	case EItemType::Normal:
	{
		const int32* FoundIndex = NormalItemIndexMap.Find(ID);
		return FoundIndex ? &NormalItems[*FoundIndex] : nullptr;
	}
	case EItemType::Weapon:
	{
		const int32* FoundIndex = WeaponItemIndexMap.Find(ID);
		return FoundIndex ? static_cast<const FZCItemTable*>(&WeaponItems[*FoundIndex]) : nullptr;
	}

	case EItemType::Shield:
	{
		const int32* FoundIndex = ShieldItemIndexMap.Find(ID);
		return FoundIndex ? static_cast<const FZCItemTable*>(&ShieldItems[*FoundIndex]) : nullptr;
	}
		
	default:
		return nullptr;
	}
}

const FZCItemTable* UZCItemGISubsystem::GetNormalItemTable(const int32 ID) const
{
	const int32* FoundIndex = NormalItemIndexMap.Find(ID);
	return FoundIndex ? &NormalItems[*FoundIndex] : nullptr;
}

const FZCWeaponTable* UZCItemGISubsystem::GetWeaponTable(const int32 ID) const
{
	const int32* FoundIndex = WeaponItemIndexMap.Find(ID);
	return FoundIndex ? &WeaponItems[*FoundIndex] : nullptr;
}

const FZCShieldTable* UZCItemGISubsystem::GetShieldTable(const int32 ID) const
{
	const int32* FoundIndex = ShieldItemIndexMap.Find(ID);
	return FoundIndex ? &ShieldItems[*FoundIndex] : nullptr;
}
