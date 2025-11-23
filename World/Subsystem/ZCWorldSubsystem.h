// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameData/Table/ZCItemTable.h"
#include "Actor/Item/Weapon/ZCWeaponActor.h"
#include "Utils/Pool/ZCPoolingClass.h"
#include "Engine/StreamableManager.h"
#include "GameData/Table/ChemistrySystemTable.h"
#include "GameData/Table/ChemistrySystemCharacterTable.h"
#include "ZCWorldSubsystem.generated.h"

class UNiagaraSystem;
struct FGameplayTag;
struct FElementCDO;
class UZCChemistryGISubsystem;
class UZCMaterialStateComponent;

DECLARE_STATS_GROUP(TEXT("Zelda"), STATGROUP_Zelda, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("WorldSubsystem Tick"), STAT_WorldSubsystem_Tick, STATGROUP_Zelda);
DECLARE_CYCLE_STAT(TEXT("WorldSubsystem Tick/ProcessSpreading"), STAT_WorldSubsystem_ProcessSpreading, STATGROUP_Zelda);

DECLARE_MULTICAST_DELEGATE(FOnAssetLoad);

UCLASS()
class ZELDA_API UZCWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void PostInitialize() override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override {
		RETURN_QUICK_DECLARE_CYCLE_STAT(UZCWorldSubsystem, STATGROUP_Tickables);
	}

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
	// MaterialStateComponent 관리
	FMaterialHandle RegisterMaterial(AZCActor* Owner, UZCNiagaraComponent* VFXComp, FGameplayTag MaterialTag);
	void UnregisterMaterial(FMaterialHandle Handle);

	// MaterialStateComponent가 호출
	void ApplyElementExposure(FMaterialHandle Handle, const FElementInfo& NewElementInfo);
	void NotifyHit(FMaterialHandle Handle, const FVector& Location, const FVector& Direction);

	// Getter
	const FMaterialRuntimeData* GetData(FMaterialHandle Handle) const;
	FGameplayTag GetMaterialTag(FMaterialHandle Handle) const;

#if WITH_EDITOR
	// 디버그용: 반응식과 임계치를 무시하고 강제로 상태를 적용
	void DebugForceSetElement(FMaterialHandle Handle, const FReactionOut& ForcedReaction);
#endif // WITH_EDITOR

protected:
	// 실제 로직 구현부
	void ProcessReaction(FMaterialRuntimeData& Data, const FElementInfo& NewInfo);
	void ApplyElement(FMaterialRuntimeData& Data, const FReactionOut& ReactionResult, int32 NewSpreadCount);
	void ProcessSpreading(FMaterialRuntimeData& Data);
	void UpdateFX(FMaterialRuntimeData& Data, bool bStart, bool bLoop, bool bEnd);

private:
	// --- Data Oriented Storage ---
	UPROPERTY()
	TArray<FMaterialRuntimeData> MaterialDataArray;

	// Handle ID -> Array Index 매핑
	TMap<int32, int32> HandleToIndexMap;

	// Array Index -> Handle ID 매핑 (Swap시 역참조용)
	TArray<int32> IndexToHandleMap;

	// 다음 발급할 핸들 ID
	int32 NextHandleID = 0;

public:
	bool TryGetObjectOutCome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FReactionOut& Out) const;
	bool TryGetCharacterOutcome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FCharacterReactionOut& Out) const;

	FORCEINLINE const FElementInstanceData* GetObjectElementInstanceData(const FGameplayTag& Element) const { return ObjectElementMap.Find(Element); }

	FORCEINLINE const FMaterialInstanceData* GetObjectMaterialInstanceData(const FGameplayTag& Material) const { return ObjectMaterialMap.Find(Material); }

	FORCEINLINE const FCharacterElementInstanceData* GetCharacterElementInstanceData(const FGameplayTag& CElement) const { return CharacterElementMap.Find(CElement); }

	FORCEINLINE const FCharacterArmorTypeInstanceData* GetCharacterMonsterTypeInstanceData(const FGameplayTag& CMonster) const { return CharacterMonsterTypeMap.Find(CMonster); }

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

