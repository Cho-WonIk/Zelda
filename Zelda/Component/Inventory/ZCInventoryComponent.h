// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameData/Table/ZCItemTable.h"
#include "UI/Common/ItemListEntry/ZCShortcutListObject.h"
#include "ZCInventoryComponent.generated.h"

class AZCItemActor;
class AZCWeaponActor;
class AZCShieldActor;

using ZCItemID = int32;
using ZCItemCount = uint8;

class UZCItemGISubsystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	// HUD 용 인벤토리 정보 반환
	[[nodiscard]] TArray<UZCShortcutListObject*> ShowShortcutInventory(const EItemType Type);
	

public:
	// 아이템 추가 함수
	bool AddItem(TUniquePtr<struct FZCItemTable> NewItem);

	// 아이템 드랍 함수(Inventory -> World)
	void DropItem(const EItemType Type, const int32 Index);

	// 아이템 제거 함수
	void RemoveItem(const EItemType Type, const FZCItemTable& RemoveItem, const int32 Index);

	// 아이템 스폰 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Spawn")
	AZCItemActor* SpawnNormalItem(const int32 Index, const FTransform& Transform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Spawn")
	AZCWeaponActor* SpawnWeaponItem(const int32 Index, const FTransform& Transform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Spawn")
	AZCShieldActor* SpawnShieldItem(const int32 Index, const FTransform& Transform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

protected:
	bool AddWeaponItem(TUniquePtr<struct FZCWeaponTable> NewWeapon);
	bool AddShieldItem(TUniquePtr<struct FZCShieldTable> NewShield);
	bool AddNormalItem(TUniquePtr<struct FZCItemTable> NewItem);


private:
	TMap<ZCItemID, ZCItemCount> NormalInventory; // 일반 아이템 인벤토리
	TArray<TUniquePtr<struct FZCWeaponTable>, TInlineAllocator<30>> WeaponInventory; // 무기 인벤토리
	TArray<TUniquePtr<struct FZCShieldTable>, TInlineAllocator<30>> ShieldInventory; // 방패 인벤토리

private:
	class UZCItemGISubsystem* ItemGISubsystem = nullptr;
};
