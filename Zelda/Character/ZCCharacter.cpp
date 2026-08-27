// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ZCCharacter.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Component/Reaction/ZCHitReactionComponent.h"

#include "Gameplay/Damage/ZCDamage.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"

#include "Component/VFX/ZCNiagaraComponent.h"
#include "Component/State/ZCStateComponent.h"
#include "Actor/Item/Weapon/ZCWeaponActor.h"
#include "Actor/Item/Shield/ZCShieldActor.h"

#include "Gameplay/ZCGameplayFunctionLibrary.h"

#include "Development/ZCLogger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCCharacter)

FName AZCCharacter::StateComponentName(TEXT("State"));

// Sets default values
AZCCharacter::AZCCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	NiagaraComponent = CreateDefaultSubobject<UZCNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);

	PhysicsControlComponent = CreateDefaultSubobject<UPhysicsControlComponent>(TEXT("PhysicsControlComponent"));
	PhysicsControlComponent->SetupAttachment(RootComponent);

	HitReactionComponent = CreateDefaultSubobject<UZCHitReactionComponent>(TEXT("HitReactionComponent"));

	StateComponent = CreateDefaultSubobject<UZCStateComponent>(AZCCharacter::StateComponentName);

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->GravityScale = 1.75f;
	GetCharacterMovement()->MaxAcceleration = 1500.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->BrakingFriction = 0.0f;
	GetCharacterMovement()->JumpZVelocity = 500.f;

	GetCharacterMovement()->SetWalkableFloorAngle(44.0f);

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	GetCharacterMovement()->GetNavMovementProperties()->bUseFixedBrakingDistanceForPaths = true;
	GetCharacterMovement()->GetNavMovementProperties()->FixedPathBrakingDistance = 200.0f;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionProfileName(TEXT("PhysicsActor"));

	GetMesh()->SetNotifyRigidBodyCollision(true);
	GetMesh()->OnComponentHit.AddDynamic(this, &AZCCharacter::OnHit);

	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AZCCharacter::OnHit);
}

void AZCCharacter::CanGotoNextSection(const FName MontageName, const FName NextSectionName)
{
	// 하위 클래스에서 오버라이드를 통해 사용
}

void AZCCharacter::DrawEquipmentNotify(EItemType Type)
{
	switch (Type)
	{
	case EItemType::Weapon:
		EquipWeapon();
		break;

	case EItemType::Shield:
		EquipShield();
		break;

	default:
		break;
	}
}

void AZCCharacter::SheathEquipmentNotify(EItemType Type)
{
	switch (Type)
	{
	case EItemType::Weapon:
		UnEquipWeapon();
		break;

	case EItemType::Shield:
		UnEquipShield();
		break;

	default:
		break;
	}
}

void AZCCharacter::OnTraceHit(bool bCanHit, bool bAllowMultipleHit, float HitInterval)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->CanHit(bCanHit, bAllowMultipleHit, HitInterval);
	}
}

void AZCCharacter::OnParry(bool bCanParry)
{

}

void AZCCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AZCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnWeaponTypeChanged.Clear();
	OnGuardStateChanged.Clear();

	Super::EndPlay(EndPlayReason);
}

void AZCCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	HitReactionComponent->Init(*MotionWarpingComponent, *PhysicsControlComponent, *GetMesh());
	
	StateComponent->OnHealthZero.AddUObject(this, &AZCCharacter::OnDeath);
	StateComponent->OnStaggerFull.AddUObject(this, &AZCCharacter::OnStagger);

	StateComponent->SetVFXComponent(NiagaraComponent);
	StateComponent->SetOwnerCharacter(this);
}

float AZCCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (StateComponent->IsDead()) return 0.0f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	FGameplayTag DamageTypeTag = FGameplayTag::EmptyTag;

	const UZCDamageType* const DamageTypeCDO = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UZCDamageType>() : GetDefault<UZCDamageType>();

	DamageTypeTag = DamageTypeCDO ? DamageTypeCDO->DamageTypeTag : FGameplayTag::EmptyTag;

	FElementInfo ElementInfo;

	// 히트 정보
	bool bIsCritial = false;
	const FHitResult* HitResult = nullptr;

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

		// 데미지 정보, 컴포넌트 전달용
		bIsCritial = IsCritialBone(GetMesh()->FindClosestBone(ZCPointDamageEvent->HitInfo.ImpactPoint));
		HitResult = &ZCPointDamageEvent->HitInfo;

		ActualDamage = InternalTakePointDamage(ActualDamage, (FPointDamageEvent) *ZCPointDamageEvent, EventInstigator, DamageCauser);

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

		// 데미지 정보, 컴포넌트 전달용
		HitResult = (ZCRadialDamageEvent->ComponentHits.Num() > 0) ? &ZCRadialDamageEvent->ComponentHits[0] : nullptr;

		ActualDamage = InternalTakeRadialDamage(ActualDamage, (FRadialDamageEvent) *ZCRadialDamageEvent, EventInstigator, DamageCauser);

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

	if (ActualDamage != 0.0f)
	{
		ReceiveAnyDamage(ActualDamage, DamageTypeCDO, EventInstigator, DamageCauser);
		OnTakeAnyDamage.Broadcast(this, ActualDamage, DamageTypeCDO, EventInstigator, DamageCauser);
		if (EventInstigator != nullptr)
		{
			EventInstigator->InstigatedAnyDamage(ActualDamage, DamageTypeCDO, this, DamageCauser);
		}
	}

	// 0.0인경우 엘리먼트 만 전파됨, 엘리먼트에 의한 데미지는 StateComponent에서 처리
	

	// 스탯 컴포넌트에 데미지 전달
	// 데미지, 원소 태그, 데미지 타입
	ActualDamage = StateComponent->ApplyDamage(ActualDamage, ElementInfo, bIsCritial);

	// 라이트		: 일반 공격
	// 미디엄		: 약점 속성
	// 헤비			: 크리티컬 공격
	// 폭발			: 폭발 또는 그로기 수치 도달

	if (HitResult != nullptr && StateComponent->IsCanPlayElementStaggerMontage() || ActualDamage != 0.0f)
	{
		EHitStrength HitStrength = EHitStrength::Light;

		if (StateComponent->IsWeak(ElementInfo.ElementTag)) HitStrength = EHitStrength::Medium;

		if (bIsCritial) HitStrength = EHitStrength::Heavy;

		if (DamageTypeTag == TAG_DamageType_Explosion || StateComponent->IsDamageStaggerFull()) HitStrength = EHitStrength::Explosion;

		HitReactionComponent->PerformHitReaction(DamageCauser, *HitResult, HitStrength, StateComponent->IsDead(), false);
	}

	return ActualDamage;
}

float AZCCharacter::InternalTakeRadialDamage(float Damage, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return Super::InternalTakeRadialDamage(Damage, RadialDamageEvent, EventInstigator, DamageCauser);
}

float AZCCharacter::InternalTakePointDamage(float Damage, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return Super::InternalTakePointDamage(Damage, PointDamageEvent, EventInstigator, DamageCauser);
}

void AZCCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (StateComponent->IsElementStaggerActive())
	{
		const FTransform& ComTransform = GetMesh()->GetComponentTransform();
		
		FVector LocalHitLocation = ComTransform.InverseTransformPosition(Hit.ImpactPoint);
		FVector LocalArrowDir = ComTransform.InverseTransformVectorNoScale(-Hit.ImpactNormal);

		StateComponent->NotifyHit(LocalHitLocation, LocalArrowDir);
	}
}

void AZCCharacter::OnDeath()
{
	UZCLogger::Warning(TEXT("Character {0} has died."), *GetName());
	bIsDead = true;
}

void AZCCharacter::OnStagger()
{
	UZCLogger::Warning(TEXT("Character {0} is staggered."), *GetName());
}

void AZCCharacter::GotoNextMontageSection(const FName NextMontageName)
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
		if (CurrentMontage && AnimInstance->Montage_IsPlaying(CurrentMontage))
		{
			AnimInstance->Montage_JumpToSection(NextMontageName, CurrentMontage);
		}
	}
}

void AZCCharacter::AttachActorToSocket(const FName SocketName, AActor* Item)
{
	if (Item)
	{
		Item->SetOwner(this);
		Item->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	}
}

void AZCCharacter::DetachActorFromSocket(AActor* Item)
{
	if (Item)
	{
		Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Item->SetOwner(nullptr);
	}
}

void AZCCharacter::SetNewWeapon(AZCWeaponActor* NewWeapon)
{
	// 무기 장착 해제 로직, nullptr이 들어온 경우
	if (!NewWeapon)
	{
		if (CurrentWeapon)
		{
			bIsEquipWeapon = false;
			DetachActorFromSocket(CurrentWeapon);
			OnWeaponTypeChanged.Broadcast(EWeaponType::None);
			CurrentWeapon->Destroy();
			CurrentWeapon = nullptr;
		}
		return;
	}

	if (CurrentWeapon && CurrentWeapon != NewWeapon)
	{
		CurrentWeapon->Destroy();
	}

	CurrentWeapon = NewWeapon;

	// 무기를 장비 중 이거나 방패를 손에 들고 있으면 손에 장착
	(bIsEquipWeapon || bIsEquipShield) ? EquipWeapon() : UnEquipWeapon();
}

void AZCCharacter::SetNewShield(AZCShieldActor* NewShield)
{
	// 방패 장착 해제 로직, nullptr이 들어온 경우
	if (!NewShield)
	{
		if (CurrentShield)
		{
			bIsEquipShield = false;
			DetachActorFromSocket(CurrentShield);
			OnShieldTypeChanged.Broadcast(EShieldType::None);
			CurrentShield->Destroy();
			CurrentShield = nullptr;
		}
		return;
	}

	if (CurrentShield)
	{
		CurrentShield->Destroy();
	}

	CurrentShield = NewShield;

	bIsEquipWeapon ? EquipShield() : UnEquipShield();
}

void AZCCharacter::EquipWeapon()
{
	if (CurrentWeapon)
	{
		AttachActorToSocket(BoneSocket::Weapon::Hand, CurrentWeapon);
		bIsEquipWeapon = true;
		OnWeaponTypeChanged.Broadcast(CurrentWeapon->GetWeaponType());
		
		// 한손검이 아닌 경우 방패를 장착 해제
		if (CurrentWeapon->GetWeaponType() != EWeaponType::OneHandSword)
		{
			UnEquipShield();
		}
	}
}

void AZCCharacter::UnEquipWeapon()
{
	if (CurrentWeapon)
	{
		AttachActorToSocket(BoneSocket::Weapon::Holster, CurrentWeapon);
		bIsEquipWeapon = false;
		OnWeaponTypeChanged.Broadcast(EWeaponType::None);
	}
}

void AZCCharacter::EquipShield()
{
	if (CurrentShield)
	{
		// 무기가 없는 상태이거나, 무기가 한손검인 경우 손에 방패를 장착
		if (!CurrentWeapon || (CurrentWeapon && CurrentWeapon->GetWeaponType() == EWeaponType::OneHandSword))
		{
			AttachActorToSocket(BoneSocket::Shield::Hand, CurrentShield);
			bIsEquipShield = true;
			OnShieldTypeChanged.Broadcast(EShieldType::Shield);
		}
		else
		{
			UnEquipShield();
		}
	}
}

void AZCCharacter::UnEquipShield()
{
	if (CurrentShield)
	{
		AttachActorToSocket(BoneSocket::Shield::Holster, CurrentShield);
		bIsEquipShield = false;
		OnShieldTypeChanged.Broadcast(EShieldType::None);
	}
}
