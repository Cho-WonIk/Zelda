// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ZCActor.h"

#include "Development/ZCLogger.h"
#include "Gameplay/Damage/ZCDamage.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Component/VFX/ZCNiagaraComponent.h"
#include "Component/Chemistry/ZCMaterialStateComponent.h"
#include "World/Subsystem/ZCWorldSubsystem.h"


// Sets default values
AZCActor::AZCActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	

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
		MaterialStateComponent->ApplyElementExposure(ElementInfo);
	}

	return ActualDamage;
}

