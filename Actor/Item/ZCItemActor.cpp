// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Item/ZCItemActor.h"
#include "Components/SphereComponent.h"
#include "Player/ZCPlayerController.h"
#include "GameData/PrimaryData/Item/ZCItemPrimaryData.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Development/ZCLogger.h"
#include "Component/VFX/ZCNiagaraComponent.h"

UMaterialInterface* AZCItemActor::DefaultOutlineMaterial = nullptr;

AZCItemActor::AZCItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	if (DefaultOutlineMaterial == nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("오버레이 머티리얼 생성자 호출 카운트"));
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> OutlineMaterialObject(TEXT("/Game/ZeldaClone/Material/M_Outline.M_Outline"));
		if (OutlineMaterialObject.Succeeded())
		{
			DefaultOutlineMaterial = OutlineMaterialObject.Object;
		}
	}

	if (!OutlineMaterial)
	{
		OutlineMaterial = DefaultOutlineMaterial;
	}

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(Zelda::Profile::Item);
	Mesh->SetVisibility(true);

	RootComponent = Mesh;

	NiagaraComponent->SetupAttachment(RootComponent);

	HighlightArea = CreateDefaultSubobject<USphereComponent>(TEXT("HighlightArea"));
	HighlightArea->InitSphereRadius(1500.0f);
	HighlightArea->SetupAttachment(Mesh);
	HighlightArea->SetCollisionProfileName(Zelda::Profile::HighlightArea);
	HighlightArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	InteractionArea = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionArea"));
	InteractionArea->InitSphereRadius(200.0f);
	InteractionArea->SetupAttachment(HighlightArea);
	InteractionArea->SetCollisionProfileName(Zelda::Profile::InteractionTrigger);
	InteractionArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	HighlightArea->OnComponentBeginOverlap.AddDynamic(this, &AZCItemActor::OnEnterHighlightRange);
	HighlightArea->OnComponentEndOverlap.AddDynamic(this, &AZCItemActor::OnExitHighlightRange);

	InteractionArea->OnComponentBeginOverlap.AddDynamic(this, &AZCItemActor::OnEnterPickUpRange);
	InteractionArea->OnComponentEndOverlap.AddDynamic(this, &AZCItemActor::OnExitPickUpRange);
}

TUniquePtr<FZCItemTable> AZCItemActor::OnItemPickUp()
{
	TUniquePtr<FZCItemTable> Item = GetWorld()->GetSubsystem<UZCWorldSubsystem>()->TakeItemFromPool(Info->Type, ItemIndex);
	Destroy();

	return TUniquePtr<FZCItemTable>(Item.Release());
}

void AZCItemActor::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if (NewOwner)
	{
		HighlightArea->SetGenerateOverlapEvents(false);
		HighlightArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		InteractionArea->SetGenerateOverlapEvents(false);
		InteractionArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		HighlightArea->SetGenerateOverlapEvents(true);
		HighlightArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		InteractionArea->SetGenerateOverlapEvents(true);
		InteractionArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		Mesh->SetCollisionProfileName(Zelda::Profile::Item);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetSimulatePhysics(true);
	}
}

void AZCItemActor::Initialize(FZCItemTable* NewItem)
{
	Info = NewItem;

	// 프라이머리 데이터 애셋 로딩
	UZCPrimaryDataAsset* RawAsset = Info->Asset.LoadSynchronous();
	if (!RawAsset) return;
	UZCItemPrimaryData* ItemAsset = Cast<UZCItemPrimaryData>(RawAsset);
	if (!ItemAsset) return;
	UStaticMesh* LoadedMesh = ItemAsset->Mesh.LoadSynchronous();
	if (LoadedMesh)
	{
		Mesh->SetStaticMesh(LoadedMesh);
		Mesh->SetSimulatePhysics(true);
	}
}

void AZCItemActor::SetOutlineMaterial(bool bEnable)
{
	TInlineComponentArray<UMeshComponent*> MeshComponents(this);

	for (UMeshComponent* MaterialMesh : MeshComponents)
	{
		if (MaterialMesh->IsA<UStaticMeshComponent>() || MaterialMesh->IsA<USkeletalMeshComponent>())
		{
			MaterialMesh->SetOverlayMaterial(bEnable ? OutlineMaterial : nullptr);
			MaterialMesh->SetRenderCustomDepth(bEnable);
		}
	}
}

void AZCItemActor::OnEnterHighlightRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (Cast<APlayerController>(Pawn->GetController()))
		{
			SetOutlineMaterial(true);
		}
	}
}

void AZCItemActor::OnExitHighlightRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetOutlineMaterial(false);
}

void AZCItemActor::OnEnterPickUpRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (AZCPlayerController* PC = Cast<AZCPlayerController>(Pawn->GetController()))
		{
			PC->NotifyItemInRange(this);
		}
	}
}

void AZCItemActor::OnExitPickUpRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (AZCPlayerController* PC = Cast<AZCPlayerController>(Pawn->GetController()))
		{
			PC->NotifyItemOutRange(this);
		}
	}
}

