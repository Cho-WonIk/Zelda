// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ZCStaticActor.h"
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
