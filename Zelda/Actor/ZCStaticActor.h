// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ZCActor.h"
#include "ZCStaticActor.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API AZCStaticActor : public AZCActor
{
	GENERATED_BODY()
	
public:
	AZCStaticActor(const FObjectInitializer& ObjectInitializer);

public:
	/*===========생성 함수===========*/
	virtual void Initialize(FZCActorTable* NewInfo) override;

	/*=======오버라이드 함수=========*/
	virtual void SetOwner(AActor* NewOwner) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> StaticMesh;
};
