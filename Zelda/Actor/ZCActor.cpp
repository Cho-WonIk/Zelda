// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ZCActor.h"
#include "Components/ShapeComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "Development/ZCLogger.h"
#include "Gameplay/Damage/ZCDamage.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Component/VFX/ZCNiagaraComponent.h"
#include "Component/Chemistry/ZCMaterialStateComponent.h"
#include "World/Subsystem/ZCWorldSubsystem.h"

#include "Settings/World/ZCWorldSettings.h"

#include "Interface/ZCDeviceInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCActor)


FName AZCActor::MeshComponentName(TEXT("Mesh"));
FName AZCActor::InteractionAreaName(TEXT("InteractionArea"));

// Sets default values
AZCActor::AZCActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<UMeshComponent>(AZCActor::MeshComponentName);
	RootComponent = Mesh;
	if (Mesh)
	{
		Mesh->SetNotifyRigidBodyCollision(true);
		Mesh->OnComponentHit.AddDynamic(this, &AZCActor::OnHit);
	}

	InteractionArea = CreateDefaultSubobject<UShapeComponent>(AZCActor::InteractionAreaName);
	if (InteractionArea)
	{
		//InteractionArea->SetupAttachment(RootComponent);
		InteractionArea->SetGenerateOverlapEvents(true);
		InteractionArea->SetCollisionProfileName(Zelda::Profile::InteractionTrigger);
		InteractionArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		InteractionArea->SetUsingAbsoluteScale(true);
		InteractionArea->OnComponentBeginOverlap.AddDynamic(this, &AZCActor::OnEnterRange);
		InteractionArea->OnComponentEndOverlap.AddDynamic(this, &AZCActor::OnExitRange);
	}

	if (Mesh && InteractionArea)
	{
		InteractionArea->SetupAttachment(RootComponent);
	}

	NiagaraComponent = CreateDefaultSubobject<UZCNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);

	MaterialStateComponent = CreateDefaultSubobject<UZCMaterialStateComponent>(TEXT("MaterialStateComponent"));

}

void AZCActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MaterialStateComponent->SetUZCNiagaraComponent(NiagaraComponent);
	MaterialStateComponent->SetCachedOwner(this);
}

float AZCActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	FGameplayTag DamageTypeTag = FGameplayTag::EmptyTag;

	FElementInfo ElementInfo;

	const UZCDamageType* const DamageTypeCDO = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UZCDamageType>() : GetDefault<UZCDamageType>();

	DamageTypeTag = DamageTypeCDO ? DamageTypeCDO->DamageTypeTag : FGameplayTag::EmptyTag;

	if (DamageEvent.IsOfType(FZCDamageEvent::ClassID))
	{
		const FZCDamageEvent* ZCDamageEvent = static_cast<const FZCDamageEvent*>(&DamageEvent);
		ElementInfo.ElementTag = ZCDamageEvent->ElementTag;
		ElementInfo.Duration = ZCDamageEvent->ElementDuration;
		ElementInfo.SpreadCount = ZCDamageEvent->ElementSpreadingCount;

	}
	else if (DamageEvent.IsOfType(FZCPointDamageEvent::ClassID))
	{
		const FZCPointDamageEvent* ZCPointDamageEvent = static_cast<const FZCPointDamageEvent*>(&DamageEvent);
		ElementInfo.ElementTag = ZCPointDamageEvent->ElementTag;
		ElementInfo.Duration = ZCPointDamageEvent->ElementDuration;
		ElementInfo.SpreadCount = ZCPointDamageEvent->ElementSpreadingCount;

		ActualDamage = InternalTakePointDamage(ActualDamage, (FPointDamageEvent)*ZCPointDamageEvent, EventInstigator, DamageCauser);

		if (ActualDamage != 0.f)
		{
			ReceivePointDamage(ActualDamage, DamageTypeCDO, ZCPointDamageEvent->HitInfo.Location, ZCPointDamageEvent->HitInfo.ImpactNormal, ZCPointDamageEvent->HitInfo.Component.Get(), ZCPointDamageEvent->HitInfo.BoneName, ZCPointDamageEvent->ShotDirection, EventInstigator, DamageCauser, ZCPointDamageEvent->HitInfo);
			OnTakePointDamage.Broadcast(this, ActualDamage, EventInstigator, ZCPointDamageEvent->HitInfo.Location, ZCPointDamageEvent->HitInfo.Component.Get(), ZCPointDamageEvent->HitInfo.BoneName, ZCPointDamageEvent->ShotDirection, DamageTypeCDO, DamageCauser);

			UPrimitiveComponent* const PrimComp = ZCPointDamageEvent->HitInfo.Component.Get();
			if (PrimComp)
			{
				PrimComp->ReceiveComponentDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
			}
		}
	}
	else if (DamageEvent.IsOfType(FZCRadialDamageEvent::ClassID))
	{
		const FZCRadialDamageEvent* ZCRadialDamageEvent = static_cast<const FZCRadialDamageEvent*>(&DamageEvent);

		ElementInfo.ElementTag = ZCRadialDamageEvent->ElementTag;
		ElementInfo.Duration = ZCRadialDamageEvent->ElementDuration;
		ElementInfo.SpreadCount = ZCRadialDamageEvent->ElementSpreadingCount;

		ActualDamage = InternalTakeRadialDamage(ActualDamage, (FRadialDamageEvent)*ZCRadialDamageEvent, EventInstigator, DamageCauser);

		if (ActualDamage != 0.f)
		{
			FHitResult const& Hit = (ZCRadialDamageEvent->ComponentHits.Num() > 0) ? ZCRadialDamageEvent->ComponentHits[0] : FHitResult();
			ReceiveRadialDamage(ActualDamage, DamageTypeCDO, ZCRadialDamageEvent->Origin, Hit, EventInstigator, DamageCauser);
			OnTakeRadialDamage.Broadcast(this, ActualDamage, DamageTypeCDO, ZCRadialDamageEvent->Origin, Hit, EventInstigator, DamageCauser);

			for (int HitIdx = 0; HitIdx < ZCRadialDamageEvent->ComponentHits.Num(); ++HitIdx)
			{
				FHitResult const& CompHit = ZCRadialDamageEvent->ComponentHits[HitIdx];
				UPrimitiveComponent* const PrimComp = CompHit.Component.Get();
				if (PrimComp && PrimComp->GetOwner() == this)
				{
					PrimComp->ReceiveComponentDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
				}
			}
		}
	}

	if (bCanElementalReaction && ElementInfo.ElementTag != FGameplayTag::EmptyTag)
	{
		MaterialStateComponent->NotifyTakeDamageExposure(ElementInfo);
		GetElement(ElementInfo.ElementTag);

	}

	return ActualDamage;
}

void AZCActor::Initialize(FZCActorTable* NewInfo)
{
	Info = NewInfo;
	MaterialStateComponent->SetMaterial(Info->MaterialTag);

	UZCActorPrimaryDataAsset* RawAsset = NewInfo->Asset.LoadSynchronous();
	if (!RawAsset) return;

	Shape = RawAsset->Shape;
}

void AZCActor::RequestOverlayState(EOverlayState StateToRequest)
{
	EOverlayState FilteredRequest = StateToRequest & (EOverlayState)CanShowOverlayState;

	if (FilteredRequest == EOverlayState::None) return;

	const EOverlayState OldState = CurrentOverlayState;
	CurrentOverlayState |= FilteredRequest;

	if (OldState != CurrentOverlayState)
	{
		UpdateOverlayMaterial();
	}
}

void AZCActor::ReleaseOverlayState(EOverlayState StateToRelease)
{
	const EOverlayState OldState = CurrentOverlayState;
	CurrentOverlayState &= ~StateToRelease; // 비트 제거

	// 만약 UltraHand 가 해제되었다면, 선택 상태도 함께 해제
	if (EnumHasAllFlags(StateToRelease, EOverlayState::UltraHand))
	{
		CurrentOverlayState &= ~EOverlayState::SelectUltraHand;
	}

	if (OldState != CurrentOverlayState)
	{
		UpdateOverlayMaterial();
	}
}

void AZCActor::SetGrabState(bool bGrab, IZCUltrahandDeviceInterface* NewOwner)
{
	UltraHandInterface = NewOwner;
	if (bGrab)
	{
		EnumAddFlags(ZCActorState, EZCActorState::Grab);
		RefreshAssembleCandidates();
	}
	else
	{
		EnumRemoveFlags(ZCActorState, EZCActorState::Grab);
	}
}

void AZCActor::OnEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (UltraHandInterface && EnumHasAllFlags(ZCActorState, EZCActorState::Grab))
	{
		if (AZCActor* ZCOtherActor = Cast<AZCActor>(OtherActor))
		{
			UltraHandInterface->AddAssembleCandidate(this, ZCOtherActor);
		}
	}
}

void AZCActor::OnExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UltraHandInterface && EnumHasAllFlags(ZCActorState, EZCActorState::Grab))
	{
		if (AZCActor* ZCOtherActor = Cast<AZCActor>(OtherActor))
		{
			UltraHandInterface->RemoveAssembleCandidate(this, ZCOtherActor);
		}
	}
}

void AZCActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (MaterialStateComponent->HasActiveElement())
	{
		MaterialStateComponent->NotifyHit(Hit.ImpactPoint, -Hit.ImpactNormal);
	}
}

void AZCActor::RefreshAssembleCandidates()
{
	if (!EnumHasAllFlags(ZCActorState, EZCActorState::Grab) || !UltraHandInterface) return;

	TArray<AActor*> OverlappingActors;
	// 현재 InteractionArea에 겹쳐 있는 모든 AZCActor를 가져옵니다.
	InteractionArea->GetOverlappingActors(OverlappingActors, AZCActor::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == this) continue;

		if (AZCActor* ZCOtherActor = Cast<AZCActor>(Actor))
		{
			UltraHandInterface->AddAssembleCandidate(this, ZCOtherActor);
		}
	}
}

void AZCActor::UpdateOverlayMaterial()
{
	const UZCWorldSettings* Settings = GetDefault<UZCWorldSettings>();

	// 1) UltraHand 상태
	if (EnumHasAllFlags(CurrentOverlayState, EOverlayState::UltraHand))
	{
		if (!IsValid(UltraHandMID))
		{
			UltraHandMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Settings->UltraHandSelectable.LoadSynchronous());
		}

		// SelectUltraHand 상태에 따라 색상 결정
		const bool bIsSelected = EnumHasAllFlags(CurrentOverlayState, EOverlayState::SelectUltraHand);

		const FLinearColor ColorToApply = bIsSelected ? Settings->UltraHandSelectedColor : Settings->UltraHandDefaultColor;

		UltraHandMID->SetVectorParameterValue(Settings->UltraHandColorParameterName, ColorToApply);

		SetOutlineMaterial(UltraHandMID);
	}
	// 2) ItemHighlight 상태
	else if (EnumHasAllFlags(CurrentOverlayState, EOverlayState::ItemHighlight))
	{
		if (!IsValid(ItemOverlayMID))
		{
			ItemOverlayMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Settings->UnacquiredItemOverlay.LoadSynchronous());
		}
		SetOutlineMaterial(ItemOverlayMID);
	}
	else
	{
		SetOutlineMaterial(nullptr);
	}
}

void AZCActor::SetOutlineMaterial(UMaterialInterface* NewMaterial)
{
	CurrentOverlayMaterial = NewMaterial;

	TInlineComponentArray<UMeshComponent*> MeshComponents(this);

	for (UMeshComponent* MaterialMesh : MeshComponents)
	{
		if (MaterialMesh->IsA<UStaticMeshComponent>() || MaterialMesh->IsA<USkeletalMeshComponent>())
		{
			MaterialMesh->SetOverlayMaterial(NewMaterial);
			MaterialMesh->SetRenderCustomDepth(NewMaterial ? true : false);
		}
	}
}

