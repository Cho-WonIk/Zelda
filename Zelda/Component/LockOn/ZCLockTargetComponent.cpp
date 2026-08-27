// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LockOn/ZCLockTargetComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

#include "ZCLockOnTrace.h"
#include "Component/LockOn/ZCLockSpringArmComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCLockTargetComponent)

UZCLockTargetComponent::UZCLockTargetComponent()
{
	bWantsInitializeComponent = true;

	Enable();
}

void UZCLockTargetComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UZCLockTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!LockIndicatorWidgetComponent)
	{
		LockIndicatorWidgetComponent = NewObject<UWidgetComponent>(this, TEXT("LockIndicatorWidget"));
		LockIndicatorWidgetComponent->SetupAttachment(this);
		LockIndicatorWidgetComponent->RegisterComponent();
	}

	if (LockIndicatorWidgetClass)
	{
		LockIndicatorWidgetComponent->SetWidgetClass(LockIndicatorWidgetClass);
	}
	else
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			if (UZCLockSpringArmComponent* SpringArm = PlayerPawn->FindComponentByClass<UZCLockSpringArmComponent>())
			{
				if (SpringArm->GetDefaultWidgetClass())
				{
					LockIndicatorWidgetComponent->SetWidgetClass(SpringArm->GetDefaultWidgetClass());
				}
			}
		}
	}

	LockIndicatorWidgetComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	LockIndicatorWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, SpawnOffset));
	LockIndicatorWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

	if (bOverrideDefaultWidgetSetting)
	{
		LockIndicatorWidgetComponent->SetDrawSize(DrawSize);
		LockIndicatorWidgetComponent->SetDrawAtDesiredSize(bDrawAtDesiredSize);
	}
	else
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			if (UZCLockSpringArmComponent* SpringArm = PlayerPawn->FindComponentByClass<UZCLockSpringArmComponent>())
			{
				LockIndicatorWidgetComponent->SetDrawSize(SpringArm->GetDefaultWidgetDrawSize());
				LockIndicatorWidgetComponent->SetDrawAtDesiredSize(SpringArm->GetDefaultWidgetDrawAtDesiredSize());
			}
		}
	}

	LockIndicatorWidgetComponent->SetVisibility(false);

	for (UActorComponent* Component : GetOwner()->GetComponents())
	{
		if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
		{
			PrimitiveComponent->SetCollisionResponseToChannel(CameraToTarget, ECollisionResponse::ECR_Ignore);
		}
	}
}

void UZCLockTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	Disable();
}

void UZCLockTargetComponent::Enable()
{
	bIsDisabled = false;
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UZCLockTargetComponent::Disable()
{
	bIsDisabled = true;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UZCLockTargetComponent::Show()
{
	LockIndicatorWidgetComponent->SetVisibility(true);
	OnLockTargetActivate.Broadcast();
}

void UZCLockTargetComponent::Hide()
{
	LockIndicatorWidgetComponent->SetVisibility(false);
	OnLockTargetDeactivate.Broadcast();
}