// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ZCStaticActor.h"
#include "Components/ShapeComponent.h"

#include "Character/ZCCharacter.h"
#include "GameData/PrimaryData/ZCActorPrimaryDataAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCStaticActor)

AZCStaticActor::AZCStaticActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UStaticMeshComponent>(AZCActor::MeshComponentName))
{
	StaticMesh = Cast<UStaticMeshComponent>(Mesh);
}

void AZCStaticActor::Initialize(FZCActorTable* NewInfo)
{
	Super::Initialize(NewInfo);

	UStreamableRenderAsset* RawMeshAsset = NewInfo->Asset->Mesh.LoadSynchronous();
	if (RawMeshAsset->GetRenderAssetType() != EStreamableRenderAssetType::StaticMesh) return;

	UStaticMesh* LoadedMesh = Cast<UStaticMesh>(RawMeshAsset);
	StaticMesh->SetStaticMesh(LoadedMesh);
	Mesh->SetSimulatePhysics(true);
}

void AZCStaticActor::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	const UWorld* World = GetWorld();
	if (World)
	{
		const EWorldType::Type WT = World->WorldType;
		if (WT == EWorldType::Editor || WT == EWorldType::EditorPreview)
		{
			return;
		}
	}

	// 1. 소유자가 없는 경우 (바닥에 놓임)
	if (!NewOwner)
	{
		// 다시 물리 효과 활성화
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetSimulatePhysics(true);

		InteractionArea->SetGenerateOverlapEvents(true);
		InteractionArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	// 2. 소유자가 '캐릭터'인 경우 (손에 잡음)
	else if (NewOwner->IsA(AZCCharacter::StaticClass()))
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		InteractionArea->SetGenerateOverlapEvents(false);
		InteractionArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
