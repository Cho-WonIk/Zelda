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
#include "Component/VFX/ZCNiagaraComponent.h"
#include "Settings/World/ZCWorldSettings.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"

void UZCWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UZCWorldSubsystem::PostInitialize()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	ChemistryGIS = &UZCChemistryGISubsystem::Get(this);

	TotalAssetCount = PreLoadElementAssetFX();

	const UZCWorldSettings* WorldSettings = GetDefault<UZCWorldSettings>();
	WorldSettings->UltraHandSelectable.LoadSynchronous();
	WorldSettings->UnacquiredItemOverlay.LoadSynchronous();
	SpreadInterval = WorldSettings->SpreadInterval;
}

void UZCWorldSubsystem::Deinitialize()
{
	ElementAssetMap.Empty();
	ActorChemistryDataArray.Empty();

	Super::Deinitialize();
}

void UZCWorldSubsystem::Tick(float DeltaTime)
{
	// stat Zelda 입력 시 소요 시간 표시
	SCOPE_CYCLE_COUNTER(STAT_WorldSubsystem_Tick);

	// Unreal Insights에서 "UZCWorldSubsystem_Tick" 트랙
	SCOPED_NAMED_EVENT(UZCWorldSubsystem_Tick, FColor::Red);

	Super::Tick(DeltaTime);

	// DoD: 연속된 배열을 순회하며 활성 상태만 처리
	for (int32 i = 0; i < ActorChemistryDataArray.Num(); ++i)
	{
		FZCRuntimeActorChemistryState& Data = ActorChemistryDataArray[i];
		if (!Data.IsValid()) continue;

		if (Data.IsDeactive()) continue;

		// Duration 처리
		if (Data.Duration > 0.0f)
		{
			Data.Duration -= DeltaTime;

			// 지속 시간 종료
			if (Data.Duration <= 0.0f)
			{
				UpdateFX(Data, false);
				Data.ResetElementState();
				continue;
			}
		}

		// Tick Damage 처리 (지속 데미지이고, 틱 데미지가 존재할 때)
		if (!Data.bIsDamageOnce && Data.TickDamage > 0.0f)
		{
			bool bShouldApplyDamage = false;

			// case 1: 0.0f 이하인 경우 (매 프레임 적용)
			if (Data.TickInterval <= 0.0f)
			{
				bShouldApplyDamage = true;
			}
			// case 2: 간격이 설정된 경우 (타이머 계산)
			else
			{
				Data.DamageTimer -= DeltaTime;
				if (Data.DamageTimer <= 0.0f)
				{
					bShouldApplyDamage = true;

					// 타이머 재설정
					Data.DamageTimer = Data.TickInterval;
				}
			}

			// 데미지 적용 수행
			if (bShouldApplyDamage)
			{
				if (Data.Owner.IsValid())
				{

				}
			}
		}

		// 주변 전파 (Spreading) 처리
		Data.SpreadTimer -= DeltaTime;
		if (Data.SpreadTimer <= 0.0f)
		{
			ProcessSpreading(Data);

			// 다음 실행 시간 설정 (부하 분산을 위한 랜덤성 부여)
			Data.RandomizeSpreadTimer(SpreadInterval);
		}
	}
}

AActor* UZCWorldSubsystem::SpawnItemByID(const EItemType Type, const int32 ID, TSubclassOf<AZCItemActor> ItemClass, FTransform const& Transform, AActor* Owner, APawn* Instigator, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, ESpawnActorScaleMethod TransformScaleMethod)
{
	if (!ItemClass) return nullptr;

	int32 ItemIndex = INDEX_NONE;
	FZCItemTable* ItemInfo = nullptr;

	auto* GISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UZCItemGISubsystem>();
	if (!GISubsystem) return nullptr;

	AZCItemActor* ItemActor = GetWorld()->SpawnActorDeferred<AZCItemActor>(ItemClass, Transform, Owner, Instigator, CollisionHandlingOverride, TransformScaleMethod);

	TWeakObjectPtr<AZCItemActor> WeakActor(ItemActor);
	FItemOverwriteCallback OverwriteAction = [WeakActor](int32 Index)
	{
		if (AZCItemActor* StrongActor = WeakActor.Get())
		{
			StrongActor->Destroy();
		}
	};

	switch (Type)
	{
	case EItemType::Normal:
	{
		const FZCItemTable* Source = GISubsystem->GetNormalItemTable(ID);
		if (!Source) return nullptr;

		TUniquePtr<FZCItemTable> NewItem = MakeUnique<FZCItemTable>(*Source);
		ItemIndex = NormalItemPool.AddItem(MoveTemp(NewItem), OverwriteAction);
		ItemInfo = NormalItemPool[ItemIndex];
		break;
	}
	case EItemType::Weapon:
	{
		const FZCWeaponTable* Source = GISubsystem->GetWeaponTable(ID);
		if (!Source) return nullptr;

		TUniquePtr<FZCWeaponTable> NewItem = MakeUnique<FZCWeaponTable>(*Source);
		ItemIndex = WeaponItemPool.AddItem(MoveTemp(NewItem), OverwriteAction);
		ItemInfo = WeaponItemPool[ItemIndex];

		break;
	}
	case EItemType::Shield:
	{
		const FZCShieldTable* Source = GISubsystem->GetShieldTable(ID);
		if (!Source) return nullptr;

		TUniquePtr<FZCShieldTable> NewItem = MakeUnique<FZCShieldTable>(*Source);
		ItemIndex = ShieldItemPool.AddItem(MoveTemp(NewItem), OverwriteAction);
		ItemInfo = ShieldItemPool[ItemIndex];
		break;
	}
	default:
		return nullptr;
	}

	if (ItemIndex == INDEX_NONE || !ItemInfo) return nullptr;


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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FZCChemistryHandle UZCWorldSubsystem::RegisterObject(AZCActor* Owner, UZCNiagaraComponent* VFXComp, FGameplayTag MaterialTag)
{
	if (!Owner || !VFXComp) return FZCChemistryHandle();

	int32 HandleID = INDEX_NONE;

	if (ChemistryFreeStack.Num() > 0)
	{
		HandleID = ChemistryFreeStack.Pop(EAllowShrinking::No);
	}
	else
	{
		HandleID = ActorChemistryDataArray.AddDefaulted();
	}

	FZCRuntimeActorChemistryState& NewData = ActorChemistryDataArray[HandleID];
	NewData.Owner = Owner;
	NewData.VFXComponent = VFXComp;
	NewData.MaterialTag = MaterialTag;
	NewData.MaterialData = ChemistryGIS->GetMaterialMap().FindRef(MaterialTag);

	return FZCChemistryHandle{HandleID};
}

void UZCWorldSubsystem::UnregisterObject(FZCChemistryHandle Handle)
{
	if (!Handle.IsValid()) return;

	FZCRuntimeActorChemistryState& Data = ActorChemistryDataArray[Handle.ID];

	if (!Data.IsValid()) return;

	UpdateFX(Data, false);
	Data.Reset();
	
	ChemistryFreeStack.Push(Handle.ID);
}

void UZCWorldSubsystem::NotifyTakeDamageExposure(FZCChemistryHandle Handle, const FElementInfo& NewElementInfo)
{
	if (!Handle.IsValid() || !ActorChemistryDataArray.IsValidIndex(Handle.ID)) return;

	FZCRuntimeActorChemistryState& Data = ActorChemistryDataArray[Handle.ID];

	if (!Data.IsValid()) return;

	ProcessReaction(Data, NewElementInfo);
}

void UZCWorldSubsystem::NotifyHit(FZCChemistryHandle Handle, const FVector& Location, const FVector& Direction)
{
	if (!Handle.IsValid() || !ActorChemistryDataArray.IsValidIndex(Handle.ID)) return;

	FZCRuntimeActorChemistryState& Data = ActorChemistryDataArray[Handle.ID];

	if (!Data.IsValid()) return;

	const FTransform& Transform = Data.Owner->GetMesh()->GetComponentTransform();
	const FVector LocalLocation = Transform.InverseTransformPosition(Location);
	const FVector LocalDirection = Transform.InverseTransformVectorNoScale(Direction);

	Data.SurfaceHitArray.Add({ LocalLocation, LocalDirection });
}

bool UZCWorldSubsystem::TryGetOutCome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FZCReactionOut& Out) const
{
	const FZCReactionOut* Found = ChemistryGIS->FindReaction(FZCReactionKey{ SourceTag, TargetTag});
	if (Found)
	{
		Out = *Found;
		return true;
	}
	return false;
}

const FZCRuntimeActorChemistryState* UZCWorldSubsystem::GetRuntimeData(FZCChemistryHandle Handle) const
{
	if (Handle.IsValid() && ActorChemistryDataArray.IsValidIndex(Handle.ID))
	{
		const FZCRuntimeActorChemistryState& Data = ActorChemistryDataArray[Handle.ID];
		if (Data.IsValid())
		{
			return &Data;
		}
	}
	return nullptr;
}

#if WITH_EDITOR
void UZCWorldSubsystem::DebugForceElement(FZCChemistryHandle Handle, const FZCReactionOut& ForcedReaction)
{
	if (!Handle.IsValid()) return;

	FZCRuntimeActorChemistryState& Data = ActorChemistryDataArray[Handle.ID];

	if (!Data.IsValid()) return;

	if (ForcedReaction.ReactionElementTag == FGameplayTag::EmptyTag)
	{
		UpdateFX(Data, false);      // FX 끄기
		Data.ResetElementState();   // 원소 상태 초기화
		Data.AccumulatedThresholds.Reset(); // 누적치 초기화
		return;
	}

	ApplyElement(Data, ForcedReaction, 10);
}
#endif // WITH_EDITOR

void UZCWorldSubsystem::ProcessReaction(FZCRuntimeActorChemistryState& Data, const FElementInfo& NewInfo)
{
	// 규칙
	// 1. 모든 전파는 Source에서 Target으로, 양방향 혹은 역전파는 일어나지 않는다.
	// 2. 우선순위 : 1순위(원소->원소) 2순위(원소->물질)
	// 3. 원소->물질의 경우 FZCSubstanceValue에 정의된 각 원소별 임계치를 초과해야 발생하게된다. 그전까지는 지속적으로 값을 주입
	// 4. 원소->원소의 경우 소스의 원소가 타겟의 원소에 영향을 준다.
	// 5. 확산 될때 SpreadingCount를 감소시키며 0에 도달하면 전파될 수 없다.(FZCElementValue에 정의된 값을 넘어서 전파될 수 없다)(확산 중단)
	// 6. 확산 방식은 지속적으로 WorldSubsystem에서 전파
	// 7. 만약 규칙이 정해지지 않은 반응이면 아무런 효과도 없다.
	// 8. 확산 카운트와 경과시간을 통해 의도치 않은 역전파를 막는 로직을 추가
	// 9. bIsDamageOnce가 true여도 만약 원소의 지속성이 있으면 Tick을 통해 원소 전파가 가능함, 데미지와 함께 전달되지 않음

	if (!ChemistryGIS) return;
	if (NewInfo.SpreadCount < 0) return;

	if (!Data.MaterialData) return;

	FZCReactionOut Out;

	// 1) 원소 -> 원소 반응
	if (Data.IsActive())
	{
		// 같은 원소를 다시 받은 경우
		if (Data.CurrentElementTag == NewInfo.ElementTag)
		{
			// 역전파 방지
			if (Data.SpreadCount > NewInfo.SpreadCount) return;

			if (Data.Duration >= 0.0f && NewInfo.Duration >= 0.0f)
			{
				Data.Duration = NewInfo.Duration;
			}
			else
			{
				// 디버그 모드
				if (NewInfo.ElementTag == TAG_Element_Fire)
				{
					Data.Duration = 10.0f;
				}
				if (NewInfo.ElementTag == TAG_Element_Electric)
				{
					Data.Duration = 1.0f;
				}
			}
		}

		if (!TryGetOutCome(NewInfo.ElementTag, Data.CurrentElementTag, Out)) return;

		// 상쇄
		if (Out.ReactionElementTag == FGameplayTag::EmptyTag)
		{
			UpdateFX(Data, false);
			Data.ResetElementState();
			return;
		}
	}
	// 2) 원소 -> 물질 반응 (임계치)
	else
	{
		if (!TryGetOutCome(NewInfo.ElementTag, Data.MaterialTag, Out)) return;

		if (Out.ReactionElementTag == FGameplayTag::EmptyTag) return;

		const float Threshold = Data.MaterialData->Threshold.FindRef(Out.ReactionElementTag);
		if (Threshold <= 0) return;

		float& CurrentValue = Data.AccumulatedThresholds.FindOrAdd(Out.ReactionElementTag);
		CurrentValue += SpreadInterval;

		if (CurrentValue < Threshold) return;
	}

	ApplyElement(Data, Out, NewInfo.SpreadCount - 1);
	Data.AccumulatedThresholds.Reset();
}

void UZCWorldSubsystem::ApplyElement(FZCRuntimeActorChemistryState& Data, const FZCReactionOut& ReactionResult, int32 NewSpreadCount)
{
	const FZCElementValue* ElementData = *ChemistryGIS->GetElementMap().Find(ReactionResult.ReactionElementTag);
	if (!ElementData) return;

	Data.CurrentElementTag = ReactionResult.ReactionElementTag;
	Data.Duration = ReactionResult.Duration;
	Data.SpreadCount = NewSpreadCount;
	Data.bIsDamageOnce = ReactionResult.bIsDamageOnce;
	Data.TickInterval = ReactionResult.TickInterval;
	Data.TickDamage = ReactionResult.TickDamage;

	// 틱 데미지 타이머 초기화 (첫 데미지는 FirstDamage로 들어가므로, 틱은 Interval 이후부터 시작)
	if (!Data.bIsDamageOnce && Data.TickInterval > 0.0f)
	{
		Data.DamageTimer = Data.TickInterval;
	}
	else
	{
		Data.DamageTimer = 0.0f;
	}

	// 초기 데미지
	if (ReactionResult.FirstDamage != 0.0f && Data.Owner.IsValid())
	{
		// TODO: 데미지 적용
	}
	// FX적용
	UpdateFX(Data, true);
}

void UZCWorldSubsystem::ProcessSpreading(FZCRuntimeActorChemistryState& Data)
{
	SCOPE_CYCLE_COUNTER(STAT_WorldSubsystem_ProcessSpreading);
	SCOPED_NAMED_EVENT(UZCWorldSubsystem_ProcessSpreading, FColor::Yellow);

	if (!ChemistryGIS || Data.IsDeactive()) return;
	if (!Data.Owner.IsValid()) return;

	const FZCElementValue* ElementData = *ChemistryGIS->GetElementMap().Find(Data.CurrentElementTag);
	if (!ElementData) return;

	FElementInfo NewInfo(Data.CurrentElementTag, Data.Duration, Data.SpreadCount);

	// Element Spreading
	if (EnumHasAnyFlags(static_cast<ESpreadShapeType>(ElementData->SpreadType), ESpreadShapeType::Element))
	{
		UZCGameplayFunctionLibrary::ApplyShapeDamage(
			NewInfo, this, 0.0f,
			Data.Owner->GetMesh()->GetComponentTransform(),
			ElementData->SpreadShape,
			UZCDamageType::StaticClass(),
			{ Data.Owner.Get() }, Data.Owner.Get(),
			Data.Owner->GetInstigatorController(),
			Zelda::Channel::Damage);
	}

	// Object Spreading
	if (EnumHasAllFlags(static_cast<ESpreadShapeType>(ElementData->SpreadType), ESpreadShapeType::Object))
	{
		if (Data.SurfaceHitArray.Num() == 0) return;

		const FTransform& ComTransform = Data.Owner->GetMesh()->GetComponentTransform();

		for (int i = Data.SurfaceHitArray.Num() - 1; i >= 0; --i)
		{
			FZCSurfaceInfo& Info = Data.SurfaceHitArray[i];

			FVector Dir = ComTransform.TransformVectorNoScale(Info.LocalDirection).GetSafeNormal();
			FVector Start = ComTransform.TransformPosition(Info.LocalLocation);

			const bool bHit = UZCGameplayFunctionLibrary::ApplyTouchDamage(
				NewInfo, this, 0.0f, Start, Dir,
				UZCDamageType::StaticClass(),
				{ Data.Owner.Get() }, Data.Owner.Get(),
				Data.Owner->GetInstigatorController(),
				Zelda::Channel::Damage);

			if (!bHit)
			{
				Data.SurfaceHitArray.RemoveAtSwap(i, 1, EAllowShrinking::No);
			}
		}
	}

}

void UZCWorldSubsystem::UpdateFX(FZCRuntimeActorChemistryState& Data, bool bStartFX)
{
	if (!Data.VFXComponent.IsValid()) return;

	const FZCLoadedElementAssetData* ElementAsset = ElementAssetMap.Find(Data.CurrentElementTag);

	if (bStartFX && ElementAsset && ElementAsset->ObjectVisuals.VFX.IsValid())
	{
		Data.VFXComponent->SetAsset(ElementAsset->ObjectVisuals.VFX.Get());
		Data.VFXComponent->Activate(true);
	}
	else
	{
		Data.VFXComponent->Deactivate();
		Data.VFXComponent->SetAsset(nullptr);
	}
}

int32 UZCWorldSubsystem::PreLoadElementAssetFX()
{
	if (!ChemistryGIS) return -1;

	// 1. 원소 테이블 가져오기
	const TMap<FGameplayTag, const FZCElementValue*>& ElementMap = ChemistryGIS->GetElementMap();
	if (ElementMap.Num() == 0) return -1;

	TArray<FSoftObjectPath> AssetsToLoad;

	// 2. 모든 원소를 순회하며 로드할 에셋 경로 수집
	for (const auto& Pair : ElementMap)
	{
		const FZCElementValue* Val = Pair.Value;
		if (!Val) continue;

		// 2-1. 정적 오브젝트용 FX 에셋 경로 수집
		if (!Val->ObjectVisuals.VFX.IsNull()) AssetsToLoad.Add(Val->ObjectVisuals.VFX.ToSoftObjectPath());
		if (!Val->ObjectVisuals.SFX.IsNull()) AssetsToLoad.Add(Val->ObjectVisuals.SFX.ToSoftObjectPath());
		if (!Val->ObjectVisuals.ElementMaterial.IsNull()) AssetsToLoad.Add(Val->ObjectVisuals.ElementMaterial.ToSoftObjectPath());

		// 2-2. 캐릭터용 FX 에셋 경로 수집
		if (!Val->CharacterVisuals.VFX.IsNull()) AssetsToLoad.Add(Val->CharacterVisuals.VFX.ToSoftObjectPath());
		if (!Val->CharacterVisuals.SFX.IsNull()) AssetsToLoad.Add(Val->CharacterVisuals.SFX.ToSoftObjectPath());
		if (!Val->CharacterVisuals.ElementMaterial.IsNull()) AssetsToLoad.Add(Val->CharacterVisuals.ElementMaterial.ToSoftObjectPath());
	}

	if (AssetsToLoad.Num() == 0) return -1;

	// 3. 비동기 로드 요청 (기존 PreLoadAsset 함수 활용)
	return StreamableHandleArr.Add(PreLoadAsset(AssetsToLoad));
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
		const TMap<FGameplayTag, const FZCElementValue*>& ElementMap = ChemistryGIS->GetElementMap();

		// 맵 초기화
		ElementAssetMap.Reset();
		ElementAssetMap.Reserve(ElementMap.Num());

		for (const auto& Pair : ElementMap)
		{
			const FGameplayTag& Key = Pair.Key;
			const FZCElementValue* Val = Pair.Value;
			if (!Val) continue;

			FZCLoadedElementAssetData& LoadedData = ElementAssetMap.FindOrAdd(Key);

			// Helper Lambda: FZCEffectVisuals -> FZCLoadedVisual 변환
			auto FillLoadedVisual = [](const FZCEffectVisuals& Source, FZCLoadedVisual& Target)
				{
					// 로딩이 완료되었으므로 Get()을 통해 실제 객체 포인터를 획득하여 StrongPtr 생성
					if (Source.VFX.Get()) Target.VFX.Reset(Source.VFX.Get());
					if (Source.SFX.Get()) Target.SFX.Reset(Source.SFX.Get());
					if (Source.ElementMaterial.Get()) Target.Material.Reset(Source.ElementMaterial.Get());
				};

			// 정적 오브젝트용 비주얼 캐싱
			FillLoadedVisual(Val->ObjectVisuals, LoadedData.ObjectVisuals);

			// 캐릭터용 비주얼 캐싱
			FillLoadedVisual(Val->CharacterVisuals, LoadedData.CharacterVisuals);
		}

		OnAssetLoadFinished.Broadcast();
		UZCLogger::Warning(TEXT("모든 애셋 로딩 완료"));
	}

}

