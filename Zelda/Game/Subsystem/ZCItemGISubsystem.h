// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameData/Table/ZCItemTable.h"
#include "Actor/Item/ZCItemActor.h"
#include "ZCItemGISubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCItemGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UZCItemGISubsystem& Get(UObject* Context);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	const FZCItemTable* GetItemTable(const EItemType Type, const int32 ID) const;

	const FZCItemTable* GetNormalItemTable(const int32 ID) const;
	const FZCWeaponTable* GetWeaponTable(const int32 ID) const;
	const FZCShieldTable* GetShieldTable(const int32 ID) const;

private:
	TArray<FZCItemTable> NormalItems;
	// 아이템 ID, TArray 인덱스
	TMap<int32, int32> NormalItemIndexMap;

	TArray<FZCWeaponTable> WeaponItems;
	// 아이템 ID, TArray 인덱스
	TMap<int32, int32> WeaponItemIndexMap;

	TArray<FZCShieldTable> ShieldItems;
	// 아이템 ID, TArray 인덱스
	TMap<int32, int32> ShieldItemIndexMap;
};

