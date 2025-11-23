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
#include "Settings/WorldInteraction/ZCInteractionWorldSettings.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"

void UZCWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UZCWorldSubsystem::PostInitialize()
{
	UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		ChemistryGIS = &UZCChemistryGISubsystem::Get(this);

		TotalAssetCount = 1;
		PreLoadObjectElementFX();
		PreLoadCharacterElementFX();

		for (const auto& M : ChemistryGIS->GetObjectMaterialMap())
		{
			FMaterialInstanceData NewMaterial;
			NewMaterial.InitFromCDO(M.Value);

			ObjectMaterialMap.Add(M.Key, NewMaterial);
		}

		for (const auto& C : ChemistryGIS->GetCharacterTypeMap())
		{
			FCharacterArmorTypeInstanceData NewMonster;
			NewMonster.InitFromMonsterCDO(C.Value);

			CharacterMonsterTypeMap.Add(C.Key, NewMonster);
		}

		const UZCInteractionWorldSettings* InteractionWorldSettings = GetDefault<UZCInteractionWorldSettings>();
		InteractionWorldSettings->UltraHandSelectable.LoadSynchronous();
		InteractionWorldSettings->UnacquiredItemOverlay.LoadSynchronous();
	}

}

void UZCWorldSubsystem::Deinitialize()
{
	MaterialDataArray.Empty();
	HandleToIndexMap.Empty();
	IndexToHandleMap.Empty();

	Super::Deinitialize();
}

void UZCWorldSubsystem::Tick(float DeltaTime)
{
	// [설명] stat Zelda 입력 시 소요 시간이 표시됩니다.
	SCOPE_CYCLE_COUNTER(STAT_WorldSubsystem_Tick);

	// [설명] Unreal Insights에서 "UZCWorldSubsystem_Tick"이라는 이름의 트랙으로 보입니다.
	SCOPED_NAMED_EVENT(UZCWorldSubsystem_Tick, FColor::Red);

	Super::Tick(DeltaTime);

	// DoD: 연속된 배열을 순회하며 활성 상태만 처리
	for (int32 i = MaterialDataArray.Num() - 1; i >= 0; --i)
	{
		FMaterialRuntimeData& Data = MaterialDataArray[i];

		// 유효성 검증
		if (!Data.OwnerActor.IsValid() || !Data.VFXComponent.IsValid())
		{
			// 무효한 데이터 제거
			const int32 HandleID = IndexToHandleMap[i];
			HandleToIndexMap.Remove(HandleID);

			if (i != MaterialDataArray.Num() - 1)
			{
				MaterialDataArray[i] = MoveTemp(MaterialDataArray.Last());
				IndexToHandleMap[i] = IndexToHandleMap.Last();

				// 이동해온 녀석(원래 마지막에 있던 녀석)의 Handle이 가리키는 Index를 i로 갱신
				const int32 MovedHandleID = IndexToHandleMap[i];
				HandleToIndexMap[MovedHandleID] = i;
			}

			MaterialDataArray.Pop(EAllowShrinking::No);
			IndexToHandleMap.Pop(EAllowShrinking::No);
			continue;
		}

		// 활성 상태가 아니면 스킵
		if (!Data.IsActive()) continue;

		// Duration 감소
		if (Data.Duration > 0.0f)
		{
			Data.Duration = FMath::Max(0.0f, Data.Duration - DeltaTime);

			if (Data.Duration == 0.0f)
			{
				Data.ResetState();
				UpdateFX(Data, false, false, true);
				continue;
			}
		}

		// 지속 데미지 처리
		if (!Data.bIsDamageOnce && Data.ElementTickDamage > 0.0f)
		{
			// TODO: 액터에 데미지 주입
		}

		// 주변 전파
		ProcessSpreading(Data);
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

FMaterialHandle UZCWorldSubsystem::RegisterMaterial(AZCActor* Owner, UZCNiagaraComponent* VFXComp, FGameplayTag MaterialTag)
{
	if (!Owner || !VFXComp) return FMaterialHandle();

	const int32 HandleID = NextHandleID++;
	const int32 Index = MaterialDataArray.Num();

	FMaterialRuntimeData NewData;
	NewData.OwnerActor = Owner;
	NewData.VFXComponent = VFXComp;
	NewData.MaterialTag = MaterialTag;

	MaterialDataArray.Add(NewData);
	HandleToIndexMap.Add(HandleID, Index);
	// 인덱스 위치 확보 후 값 설정
	if (Index >= IndexToHandleMap.Num())
	{
		IndexToHandleMap.SetNum(Index + 1);
	}
	IndexToHandleMap[Index] = HandleID;

	return FMaterialHandle{ HandleID };
}

void UZCWorldSubsystem::UnregisterMaterial(FMaterialHandle Handle)
{
	if (!Handle.IsValid()) return;

	const int32* IndexPtr = HandleToIndexMap.Find(Handle.ID);
	if (!IndexPtr) return;

	const int32 IndexToRemove = *IndexPtr;

	// Map에서 제거
	HandleToIndexMap.Remove(Handle.ID);

	if (IndexToRemove != MaterialDataArray.Num() - 1)
	{
		MaterialDataArray[IndexToRemove] = MoveTemp(MaterialDataArray.Last());
		IndexToHandleMap[IndexToRemove] = IndexToHandleMap.Last();

		// 이동된 매핑 정보 갱신
		const int32 MovedHandleID = IndexToHandleMap[IndexToRemove];
		HandleToIndexMap[MovedHandleID] = IndexToRemove;
	}

	// 3. 마지막 요소 제거
	MaterialDataArray.Pop(EAllowShrinking::No);
	IndexToHandleMap.Pop(EAllowShrinking::No);
}

void UZCWorldSubsystem::ApplyElementExposure(FMaterialHandle Handle, const FElementInfo& NewElementInfo)
{
	if (!Handle.IsValid()) return;

	const int32* IndexPtr = HandleToIndexMap.Find(Handle.ID);
	if (!IndexPtr) return;

	FMaterialRuntimeData& Data = MaterialDataArray[*IndexPtr];
	ProcessReaction(Data, NewElementInfo);
}

void UZCWorldSubsystem::NotifyHit(FMaterialHandle Handle, const FVector& Location, const FVector& Direction)
{
	if (!Handle.IsValid()) return;

	const int32* IndexPtr = HandleToIndexMap.Find(Handle.ID);
	if (!IndexPtr) return;

	FMaterialRuntimeData& Data = MaterialDataArray[*IndexPtr];

	if (!Data.OwnerActor.IsValid()) return;

	const FTransform& Transform = Data.OwnerActor->GetMesh()->GetComponentTransform();
	const FVector LocalLocation = Transform.InverseTransformPosition(Location);
	const FVector LocalDirection = Transform.InverseTransformVectorNoScale(Direction);

	Data.SurfaceHitArray.Add({ LocalLocation, LocalDirection });
}

const FMaterialRuntimeData* UZCWorldSubsystem::GetData(FMaterialHandle Handle) const
{
	if (!Handle.IsValid()) return nullptr;

	const int32* IndexPtr = HandleToIndexMap.Find(Handle.ID);
	if (!IndexPtr) return nullptr;

	return &MaterialDataArray[*IndexPtr];
}

FGameplayTag UZCWorldSubsystem::GetMaterialTag(FMaterialHandle Handle) const
{
	const FMaterialRuntimeData* Data = GetData(Handle);
	return Data ? Data->MaterialTag : FGameplayTag::EmptyTag;
}

#if WITH_EDITOR
void UZCWorldSubsystem::DebugForceSetElement(FMaterialHandle Handle, const FReactionOut& ForcedReaction)
{
	if (!Handle.IsValid()) return;

	const int32* IndexPtr = HandleToIndexMap.Find(Handle.ID);
	if (!IndexPtr) return;

	FMaterialRuntimeData& Data = MaterialDataArray[*IndexPtr];

	// 임계치 누적 등을 무시하고 바로 ApplyElement 호출
	// SpreadCount는 테스트용이므로 넉넉하게 설정 (예: 10)
	ApplyElement(Data, ForcedReaction, 10);
}
#endif // WITH_EDITOR

void UZCWorldSubsystem::ProcessReaction(FMaterialRuntimeData& Data, const FElementInfo& NewInfo)
{

	// 규칙
		// 1. 모든 전파는 Source에서 Target으로, 양방향 혹은 역전파는 일어나지 않는다.
		// 2. 우선순위 : 1순위(원소->원소) 2순위(원소->물질)
		// 3. 원소->물질의 경우 FMaterialCDO에 정의된 각 원소별 임계치를 초과해야 발생하게된다. 그전까지는 지속적으로 값을 주입
		// 4. 원소->원소의 경우 소스의 원소가 타겟의 원소에 영향을 준다.
		// 5. 확산 될때 SpreadingCount를 감소시키며 0에 도달하면 전파될 수 없다.(FElementCDO에 정의된 값을 넘어서 전파될 수 없다)(확산 중단)
		// 6. 확산 방식은 UZCMaterialStateComponent에서 지속적으로 Sphere Trace를 통해 전파
		// 7. 만약 규칙이 정해지지 않은 반응이면 아무런 효과도 없다.
		// 8. 확산 카운트와 경과시간을 통해 의도치 않은 역전파를 막는 로직을 추가
		// 9. bIsDamageOnce가 true여도 만약 원소의 지속성이 있으면 Tick을 통해 원소 전파가 가능함, 데미지와 함께 전달되지 않음
	if (!ChemistryGIS) return;
	if (NewInfo.SpreadCount < 0) return;

	const FMaterialInstanceData* MaterialData = ObjectMaterialMap.Find(Data.MaterialTag);
	if (!MaterialData) return;

	FReactionOut Out;

	// 1) 원소 -> 원소 반응
	if (!Data.IsEmpty())
	{
		// 역전파 방지
		if (Data.CurrentElementTag == NewInfo.ElementTag && Data.SpreadingCount > NewInfo.SpreadCount) return;

		if (!TryGetObjectOutCome(NewInfo.ElementTag, Data.CurrentElementTag, Out)) return;

		// 상쇄
		if (Out.NewElementTag == FGameplayTag::EmptyTag)
		{
			Data.ResetState();
			UpdateFX(Data, false, false, true);
			return;
		}
	}
	// 2) 원소 -> 물질 반응 (임계치)
	else
	{
		if (!TryGetObjectOutCome(NewInfo.ElementTag, Data.MaterialTag, Out)) return;

		if (Out.NewElementTag == FGameplayTag::EmptyTag) return;

		const float Threshold = MaterialData->Threshold.FindRef(Out.NewElementTag);
		if (Threshold <= 0.0f) return;

		float& CurrentValue = Data.AccumulatedThresholds.FindOrAdd(Out.NewElementTag);
		CurrentValue += MaterialData->ThresholdDelta.FindRef(Out.NewElementTag);

		if (CurrentValue < Threshold) return;
	}

	// 임계 통과 -> 원소 적용
	ApplyElement(Data, Out, NewInfo.SpreadCount - 1);
	Data.AccumulatedThresholds.Reset();
}

void UZCWorldSubsystem::ApplyElement(FMaterialRuntimeData& Data, const FReactionOut& ReactionResult, int32 NewSpreadCount)
{
	const FElementInstanceData* ElementData = ObjectElementMap.Find(ReactionResult.NewElementTag);
	if (!ElementData) return;

	Data.CurrentElementTag = ReactionResult.NewElementTag;
	Data.Duration = ReactionResult.Duration;
	Data.SpreadingCount = NewSpreadCount;
	Data.bIsDamageOnce = ReactionResult.bIsDamageOnce;
	Data.ElementTickDamage = ReactionResult.ElementTickDamage;

	// 초기 데미지
	if (ReactionResult.ElementFirstDamage != 0.0f && Data.OwnerActor.IsValid())
	{
		// TODO: 데미지 적용
	}

	UpdateFX(Data, true, false, false);

	if (Data.Duration > 0.0f || Data.Duration == -1.0f)
	{
		UpdateFX(Data, false, true, false);
	}
}

void UZCWorldSubsystem::ProcessSpreading(FMaterialRuntimeData& Data)
{
	// [수정 2] stat Zelda의 하위 항목으로 표시됨
	SCOPE_CYCLE_COUNTER(STAT_WorldSubsystem_ProcessSpreading);

	// [수정 3] Unreal Insights에서 별도 구간으로 표시되도록 추가
	SCOPED_NAMED_EVENT(UZCWorldSubsystem_ProcessSpreading, FColor::Yellow);

	if (!ChemistryGIS || Data.IsEmpty()) return;
	if (!Data.OwnerActor.IsValid()) return;

	const FElementInstanceData* ElementData = ObjectElementMap.Find(Data.CurrentElementTag);
	if (!ElementData) return;

	FElementInfo NewInfo(Data.CurrentElementTag, Data.Duration, Data.SpreadingCount);

	// Element Spreading
	if (EnumHasAllFlags(static_cast<ESpreadShapeType>(ElementData->SpreadType), ESpreadShapeType::Element))
	{
		UZCGameplayFunctionLibrary::ApplyShapeDamage(
			NewInfo, this, 0.0f,
			Data.OwnerActor->GetMesh()->GetComponentTransform(),
			ElementData->SpreadShape,
			UZCDamageType::StaticClass(),
			{ Data.OwnerActor.Get() }, Data.OwnerActor.Get(),
			Data.OwnerActor->GetInstigatorController(),
			Zelda::Channel::Damage);
	}

	// Object Spreading
	if (EnumHasAllFlags(static_cast<ESpreadShapeType>(ElementData->SpreadType), ESpreadShapeType::Object))
	{
		if (Data.SurfaceHitArray.Num() == 0) return;

		const FTransform& ComTransform = Data.OwnerActor->GetMesh()->GetComponentTransform();

		for (int i = Data.SurfaceHitArray.Num() - 1; i >= 0; --i)
		{
			FZCSurfaceInfo& Info = Data.SurfaceHitArray[i];

			FVector Dir = ComTransform.TransformVectorNoScale(Info.LocalDirection).GetSafeNormal();
			FVector Start = ComTransform.TransformPosition(Info.LocalLocation);

			const bool bHit = UZCGameplayFunctionLibrary::ApplyTouchDamage(
				NewInfo, this, 0.0f, Start, Dir,
				UZCDamageType::StaticClass(),
				{ Data.OwnerActor.Get() }, Data.OwnerActor.Get(),
				Data.OwnerActor->GetInstigatorController(),
				Zelda::Channel::Damage);

			if (!bHit)
			{
				Data.SurfaceHitArray.RemoveAt(i, EAllowShrinking::Yes);
			}
		}
	}
}

void UZCWorldSubsystem::UpdateFX(FMaterialRuntimeData& Data, bool bStart, bool bLoop, bool bEnd)
{
	if (!Data.VFXComponent.IsValid()) return;

	const FElementInstanceData* ElementData = ObjectElementMap.Find(Data.CurrentElementTag);

	if (bStart && ElementData && ElementData->StartVFX)
	{
		Data.VFXComponent->SetAsset(ElementData->StartVFX);
		Data.VFXComponent->Activate(true);
	}
	else if (bLoop && ElementData && ElementData->LoopVFX)
	{
		Data.VFXComponent->SetAsset(ElementData->LoopVFX);
		Data.VFXComponent->Activate(true);
	}
	else if (bEnd)
	{
		if (ElementData && ElementData->EndVFX)
		{
			Data.VFXComponent->SetAsset(ElementData->EndVFX);
			Data.VFXComponent->Activate(true);
		}
		else
		{
			Data.VFXComponent->Deactivate();
		}
	}
	else
	{
		Data.VFXComponent->Deactivate();
	}
}

int32 UZCWorldSubsystem::PreLoadObjectElementFX()
{
	if (!ChemistryGIS) return -1;

	TArray<const FElementCDO*> Rows;
	ChemistryGIS->GetAllObjectElementCDOs(Rows);

	if (Rows.Num() == 0) return -1;

	TArray<FSoftObjectPath> Arr;

	for (const auto &CDO : Rows)
	{
		if (!CDO) continue;

		Arr.Add(CDO->StartVFX.ToSoftObjectPath());
		Arr.Add(CDO->StartSFX.ToSoftObjectPath());

		Arr.Add(CDO->LoopVFX.ToSoftObjectPath());
		Arr.Add(CDO->LoopSFX.ToSoftObjectPath());

		Arr.Add(CDO->EndVFX.ToSoftObjectPath());
		Arr.Add(CDO->EndSFX.ToSoftObjectPath());
	}

	return StreamableHandleArr.Add(PreLoadAsset(Arr));
}

int32 UZCWorldSubsystem::PreLoadCharacterElementFX()
{
	if (!ChemistryGIS) return -1;

	TArray<const FCharacterElementCDO*> Rows;
	ChemistryGIS->GetAllCharacterElementCDOs(Rows);

	if (Rows.Num() == 0) return -1;

	TArray<FSoftObjectPath> Arr;

	for (const auto &CDO : Rows)
	{
		if (!CDO) continue;

		Arr.Add(CDO->ElementMaterial.ToSoftObjectPath());
		Arr.Add(CDO->VFX.ToSoftObjectPath());
		Arr.Add(CDO->SFX.ToSoftObjectPath());
	}

	return StreamableHandleArr.Add(PreLoadAsset(Arr));
}

bool UZCWorldSubsystem::TryGetObjectOutCome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FReactionOut& Out) const
{
	const FReactionOut* Found = ChemistryGIS->FindObjectReaction(FReactionKey{SourceTag, TargetTag});
	if (Found)
	{
		Out = *Found;
		return true;
	}
	return false;
}

bool UZCWorldSubsystem::TryGetCharacterOutcome(const FGameplayTag& SourceTag, const FGameplayTag& TargetTag, FCharacterReactionOut& Out) const
{
	const FCharacterReactionOut* Found = ChemistryGIS->FindCharacterReaction(FCharacterReactionKey{SourceTag, TargetTag});
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
		for (const auto &E : ChemistryGIS->GetObjectElementMap())
		{
			FElementInstanceData NewElement;
			NewElement.InitFromCDO(E.Value);

			ObjectElementMap.Add(E.Key, NewElement);
		}

		for (const auto &E : ChemistryGIS->GetCharacterElementMap())
		{
			FCharacterElementInstanceData NewElement;
			NewElement.InitFromCDO(E.Value);

			CharacterElementMap.Add(E.Key, NewElement);
		}

		OnAssetLoadFinished.Broadcast();
		UZCLogger::Warning(TEXT("모든 애셋 로딩 완료"));
	}

}
