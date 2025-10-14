// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameData/ZCItemTable.h"
#include "Actor/Item/Weapon/ZCWeaponActor.h"
#include "Utils/Pool/ZCPoolingClass.h"
#include "Engine/StreamableManager.h"
#include "Gameplay/ChemistrySystem/ChemistrySystemTable.h"
#include "Gameplay/ChemistrySystem/ChemistrySystemCharacterTable.h"
#include "ZCWorldSubsystem.generated.h"

class UNiagaraSystem;
struct FGameplayTag;
struct FElementCDO;
class UZCChemistryGISubsystem;
class UZCMaterialStateComponent;

DECLARE_MULTICAST_DELEGATE(FOnAssetLoad);

UCLASS()
class ZELDA_API UZCWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void PostInitialize() override;
	virtual void Deinitialize() override;

public:
	FOnAssetLoad OnAssetLoadFinished;

	/*===================================================== 아이템 관련 ==========================================================*/
public:
	// 아이템 스폰
	UFUNCTION(BlueprintCallable, Category = "Zelda|Spawn")
	AActor* SpawnItemByID(
		const EItemType Type,
		const int32 ID,
		TSubclassOf<AZCItemActor> ItemClass,
		FTransform const& Transform,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr,
		ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined,
		ESpawnActorScaleMethod TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot);

	AActor* SpawnItemByData(
		FZCItemTable* ItemInfo,
		int32 Index,
		TSubclassOf<AZCItemActor> ItemClass,
		FTransform const& Transform,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr,
		ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined,
		ESpawnActorScaleMethod TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot);

	template<typename T>//, typename = std::enable_if_t<TIsDerivedFrom<T, AZCItemActor>::Value>>
	T* SpawnItemByID(const EItemType Type, const int32 ID, FTransform const& Transform, AActor* Owner = nullptr, APawn* Instigator = nullptr,
		ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined, ESpawnActorScaleMethod TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot)
	{
		return Cast<T>(SpawnItemByID(Type, ID, T::StaticClass(), Transform, Owner, Instigator, CollisionHandlingOverride, TransformScaleMethod));
	}

	template<typename T>
	T* SpawnItemByData(FZCItemTable* ItemInfo, int32 Index, FTransform const& Transform, AActor* Owner = nullptr, APawn* Instigator = nullptr,
		ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined, ESpawnActorScaleMethod TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot)
	{
		return Cast<T>(SpawnItemByData(ItemInfo, Index, T::StaticClass(), Transform, Owner, Instigator, CollisionHandlingOverride, TransformScaleMethod));
	}

	 
	/*============== 아이템 풀 ==============*/
private:
	static constexpr int32 MaxNormalPoolSize = 30;
	static constexpr int32 MaxWeaponPoolSize = 10;
	static constexpr int32 MaxShieldPoolSize = 10;

public:
	ZCFixedItemPool<FZCItemTable, MaxNormalPoolSize, TFixedAllocator<MaxNormalPoolSize>>& GetNormalItemPool() { return NormalItemPool; }
	ZCFixedItemPool<FZCWeaponTable, MaxWeaponPoolSize, TFixedAllocator<MaxWeaponPoolSize>>& GetWeaponItemPool() { return WeaponItemPool; }
	ZCFixedItemPool<FZCShieldTable, MaxShieldPoolSize, TFixedAllocator<MaxShieldPoolSize>>& GetShieldItemPool() { return ShieldItemPool; }

	[[nodiscard]] int32 AddItemToPool(const EItemType Type, TUniquePtr<FZCItemTable> NewItem);
	[[nodiscard]] TUniquePtr<FZCItemTable> TakeItemFromPool(const EItemType Type, int32 Index);
	void RemoveItemFromPool(const EItemType Type, int32 Index);

private:
	ZCFixedItemPool<FZCItemTable, MaxNormalPoolSize, TFixedAllocator<MaxNormalPoolSize>> NormalItemPool;
	ZCFixedItemPool<FZCWeaponTable, MaxWeaponPoolSize, TFixedAllocator<MaxWeaponPoolSize>> WeaponItemPool;
	ZCFixedItemPool<FZCShieldTable, MaxShieldPoolSize, TFixedAllocator<MaxShieldPoolSize>> ShieldItemPool;


	/*===================================================== 화학 반응 ==========================================================*/
public:
	bool TryGetObjectOutCome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FReactionOut& Out) const;
	bool TryGetCharacterOutcome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FCharacterReactionOut& Out) const;

	FORCEINLINE const FElementInstanceData* GetObjectElementInstanceData(const FGameplayTag& Element) const
	{
		return ObjectElementMap.Find(Element);
	}

	FORCEINLINE const FMaterialInstanceData* GetObjectMaterialInstanceData(const FGameplayTag& Material) const
	{
		return ObjectMaterialMap.Find(Material);
	}

	FORCEINLINE const FCharacterElementInstanceData* GetCharacterElementInstanceData(const FGameplayTag& CElement) const
	{
		return CharacterElementMap.Find(CElement);
	}

	FORCEINLINE const FCharacterArmorTypeInstanceData* GetCharacterMonsterTypeInstanceData(const FGameplayTag& CMonster) const
	{
		return CharacterMonsterTypeMap.Find(CMonster);
	}

protected:
	// 로드할 애셋들 
	int32 PreLoadObjectElementFX();
	int32 PreLoadCharacterElementFX();

protected:
	UPROPERTY(Transient)
	TMap<FGameplayTag, FElementInstanceData> ObjectElementMap;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FMaterialInstanceData> ObjectMaterialMap;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FCharacterElementInstanceData> CharacterElementMap;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FCharacterArmorTypeInstanceData> CharacterMonsterTypeMap;

	// TODO : 플레이어 갑옷 로직
	UPROPERTY(Transient)
	TMap<FGameplayTag, FCharacterArmorTypeInstanceData> PlayerArmorTypeMap;

	UPROPERTY(Transient)
	UZCChemistryGISubsystem* ChemistryGIS = nullptr;

	/*===================================================== 유틸 함수들 ==========================================================*/
	// 비동기 로드
private:
	TSharedPtr<FStreamableHandle> PreLoadAsset(const TArray<FSoftObjectPath>& AssetArr);

	void OnAssetLoadComplete();

	TArray<TSharedPtr<FStreamableHandle>> StreamableHandleArr;

	uint8 TotalAssetCount = 0;
	uint8 CompletedLoadingAssetCount = 0;
};

