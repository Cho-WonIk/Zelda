// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameData/Table/ZCActorTable.h"
#include "GameData/Enum/ZCItemType.h"
#include "Gameplay/Damage/ZCDamage.h"
#include "ZCItemTable.generated.h"

USTRUCT(BlueprintType)
struct FZCItemTable : public FZCActorTable
{
	GENERATED_BODY()

public:
	FZCItemTable()
		: ItemType(EItemType::None)
		, Rarity(Rarity::Common)
		, MaxCount(1)
		, bIsDestroyable(false)
		, Durability(-1)
	{
		Type = EZCActortype::Item;
		Name = FText();
		Description = FText();
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "아이템", meta = (DisplayName = "아이템 분류", DisplayPriority = 1))
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "아이템", meta = (DisplayName = "희귀도", DisplayPriority = 2))
	Rarity Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "아이템", meta = (DisplayName = "아이템 이름", DisplayPriority = 3))
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "아이템", meta = (DisplayName = "최대 개수", DisplayPriority = 4))
	int32 MaxCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "아이템", meta = (DisplayName = "파괴 가능 유무", DisplayPriority = 5))
	bool bIsDestroyable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "아이템", meta = (DisplayName = "내구도", DisplayPriority = 6))
	int32 Durability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "아이템", meta = (DisplayName = "아이템 설명", DisplayPriority = 7))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "아이템", meta = (DisplayName = "아이템 아이콘", DisplayPriority = 8))
	TSoftObjectPtr<UTexture2D> Icon;

	bool operator==(const FZCItemTable& Other) const
	{
		return FZCActorTable::operator==(Other) && ItemType == Other.ItemType && Rarity == Other.Rarity && Durability == Other.Durability;
	}
};


USTRUCT(BlueprintType)
struct FZCWeaponTable : public FZCItemTable
{
	GENERATED_BODY()

public:
	FZCWeaponTable()
		: WeaponType(EWeaponType::None)
		, AttackPower(1)
		, AttackSpeed(1.0f)
	{ ItemType = EItemType::Weapon; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "무기", meta = (DisplayName = "무기 타입", DisplayPriority = 100))
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "무기", meta = (DisplayName = "공격력", DisplayPriority = 101))
	int32 AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "무기", meta = (DisplayName = "공격 속도", DisplayPriority = 102))
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "무기", meta = (DisplayName = "무기 공격 타입", DisplayPriority = 103))
	TSubclassOf<UZCDamageType> DamageTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "무기", meta = (DisplayName = "무기 원소 타입", DisplayPriority = 104, GameplayTagFilter = "Element"))
	FGameplayTag ElementType;

	bool operator==(const FZCWeaponTable& Other) const
	{
		return FZCItemTable::operator==(Other) && WeaponType == Other.WeaponType && AttackPower == Other.AttackPower && AttackSpeed == Other.AttackSpeed && ElementType == Other.ElementType && DamageTypeTag == Other.DamageTypeTag;
	}
};


USTRUCT(BlueprintType)
struct FZCShieldTable : public FZCItemTable
{
	GENERATED_BODY()

public:
	FZCShieldTable()
	{ ItemType = EItemType::Shield; }

	bool operator==(const FZCShieldTable& Other) const
	{
		return FZCItemTable::operator==(Other);
	}
};