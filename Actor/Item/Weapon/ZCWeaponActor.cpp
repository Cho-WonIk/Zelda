// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Item/Weapon/ZCWeaponActor.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"

#include "Component/HitTrace/ZCHitTraceComponent.h"
#include "Physics/ZCCollision.h"

#include "World/Subsystem/ZCWorldSubsystem.h"

#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCWeaponActor)

AZCWeaponActor::AZCWeaponActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Mesh->SetCollisionProfileName(Zelda::Profile::Weapon);
	HitTraceComponent = CreateDefaultSubobject<UZCHitTraceComponent>(TEXT("HitTraceComponent"));
}

void AZCWeaponActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	HitTraceComponent->OnHitActor.BindUObject(this, &AZCWeaponActor::OnHitActor);
}

void AZCWeaponActor::Initialize(FZCActorTable* NewItem)
{
	Super::Initialize(NewItem);
	if (ItemInfo->ItemType != EItemType::Weapon) return;

	WeaponInfo = static_cast<FZCWeaponTable*>(Info);
	if (const FElementInstanceData* const ElementInfo = GetWorld()->GetSubsystem<UZCWorldSubsystem>()->GetObjectElementInstanceData(WeaponInfo->ElementType))
	{
		WeaponElementInfo.ElementTag = WeaponInfo->ElementType;
		WeaponElementInfo.SpreadCount = ElementInfo->MaxSpreadingCount;
	}
}

void AZCWeaponActor::CanHit(bool bCanHit, bool bAllowMultipleHit, float HitInterval)
{
	if (bCanHit)
	{
		FZCHitTraceStruct HitType = FZCHitTraceStruct::ChannelDefault;

		HitType.Mesh = Cast<USceneComponent>(Mesh);
		HitType.TraceChannel = UEngineTypes::ConvertToTraceType(Zelda::Channel::Damage);
		HitType.Instigator = this;
		HitType.IgnoreActors.Add(this);
		HitType.IgnoreActors.Add(GetOwner());

		HitType.bAllowMultipleHit = bAllowMultipleHit;
		HitType.HitInterval = HitInterval;

		HitTraceComponent->BeginTrace(FName(TEXT("Trace")), HitType);
	}
	else
	{
		HitTraceComponent->EndTrace(FName(TEXT("Trace")));
	}
}

void AZCWeaponActor::OnHitActor(AActor* HitActor, const FHitResult& HitResult)
{
	if (HitActor)
	{
		UZCGameplayFunctionLibrary::ApplyPointDamage(WeaponElementInfo, HitActor, WeaponInfo->AttackPower, HitResult.Location, HitResult, GetOwner()->GetInstigatorController(), this, WeaponInfo->DamageTypeTag);
	}
}
