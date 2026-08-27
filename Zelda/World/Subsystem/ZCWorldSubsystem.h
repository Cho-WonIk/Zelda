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
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UZCWorldSubsystem, STATGROUP_Tickables); }

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

public:
	// 오브젝트 등록 및 해제
	// 오브젝트 등록
	FZCChemistryHandle RegisterObject(AZCActor* Owner, UZCNiagaraComponent* VFXComp, FGameplayTag MaterialTag);
	// 오브젝트 해제
	void UnregisterObject(FZCChemistryHandle Handle);

	// 오브젝트에서 특정 원소를 전달 받았음을 알림
	// 액터의 TakeDamage API를 통해 받은 정보
	void NotifyTakeDamageExposure(FZCChemistryHandle Handle, const FElementInfo& NewElementInfo);
	// 표면 전파에서 접촉된 표면정보를 저장
	void NotifyHit(FZCChemistryHandle Handle, const FVector& Location, const FVector& Direction);

	// 적용 가능한 원소 검색
	bool TryGetOutCome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FZCReactionOut& Out) const;

	const FZCRuntimeActorChemistryState* GetRuntimeData(FZCChemistryHandle Handle) const;

	const TMap<FGameplayTag, FZCLoadedElementAssetData>* GetElementAssetMap() const { return &ElementAssetMap; }

#if WITH_EDITOR
	// 디버그용: 반응식과 임계치를 무시하고 강제로 상태를 적용
	void DebugForceElement(FZCChemistryHandle Handle, const FZCReactionOut& ForcedReaction);
#endif // WITH_EDITOR

protected:
	// 실제 로직 구현부
	// 물질에 원소 데미지를 누적시키고, 임계점을 넘으면 ApplyElement 호출
	void ProcessReaction(FZCRuntimeActorChemistryState& Data, const FElementInfo& NewInfo);
	// 오브젝트에 실제 원소 적용
	void ApplyElement(FZCRuntimeActorChemistryState& Data, const FZCReactionOut& ReactionResult, int32 NewSpreadCount);
	// 적용된 원소를 주변에 전파하는 로직
	void ProcessSpreading(FZCRuntimeActorChemistryState& Data);
	// 오브젝트에 FX를 적용 및 해제
	void UpdateFX(FZCRuntimeActorChemistryState& Data, bool bStartFX);

	// 로드할 애셋들
	int32 PreLoadElementAssetFX();


protected:
	// 비동기 엘리먼트 애셋 로드 강참조 유지
	TMap<FGameplayTag, FZCLoadedElementAssetData> ElementAssetMap;

	TArray<int32> ChemistryFreeStack;

	TArray<FZCRuntimeActorChemistryState> ActorChemistryDataArray;

	int32 NextChemistryHandleID = 0;

	UZCChemistryGISubsystem* ChemistryGIS = nullptr;

	/*===================================================== 유틸 함수들 ==========================================================*/
	// 비동기 로드
private:
	TSharedPtr<FStreamableHandle> PreLoadAsset(const TArray<FSoftObjectPath>& AssetArr);

	void OnAssetLoadComplete();

	TArray<TSharedPtr<FStreamableHandle>> StreamableHandleArr;

	uint8 TotalAssetCount = 0;
	uint8 CompletedLoadingAssetCount = 0;

	float SpreadInterval = 0.0f;
};

