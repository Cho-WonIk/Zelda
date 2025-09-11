// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/ZCMonsterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/WidgetComponent.h"
#include "AI/ZCAIControllerBase.h"
#include "Component/State/Monster/ZCMonsterStateComponent.h"
#include "Component/Perception/ZCAIPerceptionComponent.h"
#include "Component/LockOn/ZCLockTargetComponent.h"
#include "CrowdControl/ZCMonsterGroupActor.h"
#include "Utils/Team/ZCTeam.h"

#include "UI/Common/State/ZCBarWidget.h"

AZCMonsterCharacter::AZCMonsterCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UZCMonsterStateComponent>(AZCCharacter::StateComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	LockOnComponent = CreateDefaultSubobject<UZCLockTargetComponent>(TEXT("LockOnComponent"));
	LockOnComponent->SetupAttachment(RootComponent);

	LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
	LifeBar->SetupAttachment(RootComponent);
	LifeBar->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	LifeBar->SetWidgetSpace(EWidgetSpace::Screen);
	LifeBar->SetDrawSize(FVector2D(100, 25));

	AggroIndicator = CreateDefaultSubobject<UWidgetComponent>(TEXT("AggroIndicator"));
	AggroIndicator->SetupAttachment(RootComponent);
	AggroIndicator->SetWidgetSpace(EWidgetSpace::Screen);

	ZCMonsterStateComponent = ExactCast<UZCMonsterStateComponent>(GetStateComponent());

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -98.0f), FRotator(0.0f, -90.0f, 0.0f));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->MaxWalkSpeed = 250.0f;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> Montage1(TEXT("/Game/ZeldaClone/Animation/Monster/Idle/AM_GoblinGroupChat_F1.AM_GoblinGroupChat_F1"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Montage2(TEXT("/Game/ZeldaClone/Animation/Monster/Idle/AM_GoblinGroupChat_M1.AM_GoblinGroupChat_M1"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Montage3(TEXT("/Game/ZeldaClone/Animation/Monster/Idle/AM_GoblinGroupChat_M2.AM_GoblinGroupChat_M2"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Montage4(TEXT("/Game/ZeldaClone/Animation/Monster/Idle/AM_GoblinGroupChat_M3.AM_GoblinGroupChat_M3"));

	if (Montage1.Object) IdleTalkMontages.Add(Montage1.Object);
	if (Montage2.Object) IdleTalkMontages.Add(Montage2.Object);
	if (Montage3.Object) IdleTalkMontages.Add(Montage3.Object);
	if (Montage4.Object) IdleTalkMontages.Add(Montage4.Object);

	TeamID = 2;
}

void AZCMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	UZCBarWidget* BarWidget = CastChecked<UZCBarWidget>(LifeBar->GetUserWidgetObject());
	GetStateComponent()->OnHealthChanged.AddLambda([BarWidget](const FZCStat& State) { if (BarWidget) { BarWidget->SetBarPercent(State.GetNormalized()); } });


	UZCBarWidget* AggroBarWidget = CastChecked<UZCBarWidget>(AggroIndicator->GetUserWidgetObject());
	if (AZCAIControllerBase* AIController = Cast<AZCAIControllerBase>(GetController()))
	{
		if (UZCAIPerceptionComponent* PerceptionComp = AIController->GetZCPerceptionComponent())
		{
			PerceptionComp->OnAggroUpdate.AddLambda([AggroBarWidget](float NormalizedAggro) {
				if (AggroBarWidget) {
					AggroBarWidget->SetBarPercent(NormalizedAggro);
				}
				});
		}
	}
}

void AZCMonsterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CurrentWeapon)
	{
		
	}

	if (CurrentShield)
	{

	}

	Super::EndPlay(EndPlayReason);
}

void AZCMonsterCharacter::AttackByAI_Implementation(const FVector& TargetLocation)
{
	if (bIsDead) return;
	if (TargetLocation != FVector::ZeroVector && MotionWarpingComponent)
	{
		FMotionWarpingTarget WarpTarget;
		WarpTarget.Name = MotionWarping::WarpTarget::AttackTarget;
		WarpTarget.Location = GetActorLocation();
		WarpTarget.Rotation  = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLocation);

		MotionWarpingComponent->AddOrUpdateWarpTarget(WarpTarget);
	}

	PlayAnimMontage(AttackMontage, 1.0f);
}

void AZCMonsterCharacter::DefenseByAI_Implementation(const FVector& TargetLocation)
{
	if (bIsDead) return;

	UE_LOG(LogTemp, Warning, TEXT("AI가 방어를 함 %s"), *GetName());
}

void AZCMonsterCharacter::EquipWeaponByAI(AZCWeaponActor* Weapon)
{
	if (Weapon)
	{
		SetNewWeapon(Weapon);
		EquipWeapon();
	}
	else
	{
		SetNewWeapon(nullptr);
	}
}

void AZCMonsterCharacter::EquipShieldByAI(AZCShieldActor* Shield)
{
	if (Shield)
	{
		SetNewShield(Shield);
		EquipShield();
	}
	else
	{
		SetNewShield(nullptr);
	}
}

void AZCMonsterCharacter::OnDeath()
{
	Super::OnDeath();

	GetWorld()->GetTimerManager().SetTimer(OnDeathTimerHandle, FTimerDelegate::CreateLambda([this]() { Destroy(); }), 2.0f, false);
}
