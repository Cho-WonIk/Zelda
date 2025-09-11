// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Game/Subsystem/ZCItemGISubsystem.h"
#include "Game/Subsystem/ZCChemistryGISubsystem.h"
#include "Development/ZCLogger.h"
#include "Component/Chemistry/ZCMaterialStateComponent.h"


void UZCWorldSubsystem::PostInitialize()
{
	UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		ChemistryGIS = &UZCChemistryGISubsystem::Get(this);

		TotalAssetCount = 1;
		PreLoadElementFX();
	}

}

void UZCWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

AActor* UZCWorldSubsystem::SpawnItemByID(const EItemType Type, const int32 ID, TSubclassOf<AZCItemActor> ItemClass, FTransform const& Transform, AActor* Owner, APawn* Instigator, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, ESpawnActorScaleMethod TransformScaleMethod)
{
	if (!ItemClass) return nullptr;

	int32 ItemIndex = INDEX_NONE;
	FZCItemTable* ItemInfo = nullptr;

	auto* GISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UZCItemGISubsystem>();
	if (!GISubsystem) return nullptr;

	switch (Type)
	{
	case EItemType::Normal:
	{
		const FZCItemTable* Source = GISubsystem->GetNormalItemTable(ID);
		if (!Source) return nullptr;

		TUniquePtr<FZCItemTable> NewItem = MakeUnique<FZCItemTable>(*Source);
		ItemIndex = NormalItemPool.AddItem(MoveTemp(NewItem));
		ItemInfo = NormalItemPool[ItemIndex];
		break;
	}
	case EItemType::Weapon:
	{
		const FZCWeaponTable* Source = GISubsystem->GetWeaponTable(ID);
		if (!Source) return nullptr;

		TUniquePtr<FZCWeaponTable> NewItem = MakeUnique<FZCWeaponTable>(*Source);
		ItemIndex = WeaponItemPool.AddItem(MoveTemp(NewItem));
		ItemInfo = WeaponItemPool[ItemIndex];

		break;
	}
	case EItemType::Shield:
	{
		const FZCShieldTable* Source = GISubsystem->GetShieldTable(ID);
		if (!Source) return nullptr;

		TUniquePtr<FZCShieldTable> NewItem = MakeUnique<FZCShieldTable>(*Source);
		ItemIndex = ShieldItemPool.AddItem(MoveTemp(NewItem));
		ItemInfo = ShieldItemPool[ItemIndex];
		break;
	}
	default:
		return nullptr;
	}

	if (ItemIndex == INDEX_NONE || !ItemInfo) return nullptr;

	AZCItemActor* ItemActor = GetWorld()->SpawnActorDeferred<AZCItemActor>(ItemClass, Transform, Owner, Instigator, CollisionHandlingOverride, TransformScaleMethod);

	if (!ItemActor) return nullptr;

	ItemActor->Initialize(ItemInfo);
	ItemActor->SetItemIndex(ItemIndex);
	UGameplayStatics::FinishSpawningActor(ItemActor, Transform);

	return ItemActor;
}

AActor* UZCWorldSubsystem::SpawnItemByData(FZCItemTable* ItemInfo, int32 Index, TSubclassOf<AZCItemActor> ItemClass, FTransform const& Transform, AActor* Owner, APawn* Instigator, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, ESpawnActorScaleMethod TransformScaleMethod)
{
	if (!ItemClass) return nullptr;
	AZCItemActor* ItemActor = GetWorld()->SpawnActorDeferred<AZCItemActor>(ItemClass, Transform, Owner, Instigator, CollisionHandlingOverride, TransformScaleMethod);
	if (!ItemActor) return nullptr;
	ItemActor->Initialize(ItemInfo);
	ItemActor->SetItemIndex(Index);
	UGameplayStatics::FinishSpawningActor(ItemActor, Transform);
	return ItemActor;
}

int32 UZCWorldSubsystem::AddItemToPool(const EItemType Type, TUniquePtr<FZCItemTable> NewItem)
{
	switch (Type)
	{
	case EItemType::Normal:
		return NormalItemPool.AddItem(MoveTemp(NewItem));
	case EItemType::Weapon:
		return WeaponItemPool.AddItem(TUniquePtr<FZCWeaponTable>(static_cast<FZCWeaponTable*>(NewItem.Release())));
	case EItemType::Shield:
		return ShieldItemPool.AddItem(TUniquePtr<FZCShieldTable>(static_cast<FZCShieldTable*>(NewItem.Release())));
	default:
		return INDEX_NONE;
	}
}

TUniquePtr<FZCItemTable> UZCWorldSubsystem::TakeItemFromPool(const EItemType Type, int32 Index)
{
	switch (Type)
	{
	case EItemType::Normal:
		return NormalItemPool.TakeItem(Index);

	case EItemType::Weapon:
		return WeaponItemPool.TakeItem(Index);

	case EItemType::Shield:
		return ShieldItemPool.TakeItem(Index);

	default:
		return nullptr;
	}
}

void UZCWorldSubsystem::RemoveItemFromPool(const EItemType Type, int32 Index)
{
	switch (Type)
	{
	case EItemType::Normal:
		NormalItemPool.RemoveItem(Index);
		break;
	case EItemType::Weapon:
		WeaponItemPool.RemoveItem(Index);
		break;
	case EItemType::Shield:
		ShieldItemPool.RemoveItem(Index);
		break;
	default:
		break;
	}
}

int32 UZCWorldSubsystem::PreLoadElementFX()
{
	if (!ChemistryGIS) return -1;

	TArray<const FElementCDO*> Rows;
	ChemistryGIS->GetAllElementCDOs(Rows);

	if (Rows.Num() == 0) return -1;

	TArray<FSoftObjectPath> Arr;

	for (const FElementCDO* CDO : Rows)
	{
		if (!CDO) continue;

		Arr.Add(CDO->StartVFX.ToSoftObjectPath());
		Arr.Add(CDO->StartSFX.ToSoftObjectPath());

		Arr.Add(CDO->LoopVFX.ToSoftObjectPath());
		Arr.Add(CDO->LoopSFX.ToSoftObjectPath());

		Arr.Add(CDO->EndVFX.ToSoftObjectPath());
		Arr.Add(CDO->EndSFX.ToSoftObjectPath());
		//LoadPackForTag(CDO->Tag, *CDO);
	}

	return StreamableHandleArr.Add(PreLoadAsset(Arr));
}

bool UZCWorldSubsystem::TryGetOutCome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FReactionOut& Out) const
{
	const FReactionOut* Found = ChemistryGIS->FindReaction(FReactionKey{SourceTag, TargetTag});
	if (Found)
	{
		Out = *Found;
		return true;
	}
	return false;
}

TSharedPtr<FStreamableHandle> UZCWorldSubsystem::PreLoadAsset(const TArray<FSoftObjectPath>& AssetArr)
{
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

	return Streamable.RequestAsyncLoad(AssetArr, FStreamableDelegate::CreateUObject(this, &UZCWorldSubsystem::OnAssetLoadComplete), FStreamableManager::AsyncLoadHighPriority);
}

void UZCWorldSubsystem::OnAssetLoadComplete()
{
	++CompletedLoadingAssetCount;

	UZCLogger::Warning(TEXT("애셋 로딩 완료 카운트 {0}"), CompletedLoadingAssetCount);

	if (CompletedLoadingAssetCount >= TotalAssetCount)
	{
		for (const auto &M : ChemistryGIS->GetMaterialMap())
		{
			FMaterialInstanceData NewMaterial;
			NewMaterial.Threshold = M.Value->Threshold;
			NewMaterial.ThresholdDelta = M.Value->ThresholdDelta;

			MaterialMap.Add(M.Key, NewMaterial);

			UZCLogger::Warning(TEXT("물질 애셋 로딩 중"));
		}

		for (const auto &E : ChemistryGIS->GetElementMap())
		{
			FElementInstanceData NewElement;

			NewElement.StartSFX = E.Value->StartSFX.Get();
			NewElement.StartVFX = E.Value->StartVFX.Get();

			NewElement.LoopSFX = E.Value->LoopSFX.Get();
			NewElement.LoopVFX = E.Value->LoopVFX.Get();

			NewElement.EndSFX = E.Value->EndSFX.Get();
			NewElement.EndVFX = E.Value->EndVFX.Get();

			NewElement.MaxSpreadingCount = E.Value->MaxSpreadingCount;
			NewElement.SpreadingRange = E.Value->SpreadingRange;

			ElementMap.Add(E.Key, NewElement);
		}

		OnAssetLoadFinished.Broadcast();
		UZCLogger::Warning(TEXT("모든 애셋 로딩 완료"));
	}

}
