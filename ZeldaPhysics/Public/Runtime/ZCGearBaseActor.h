// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZCGearBaseActor.generated.h"

class UZCGearClusterComponent;
class UZCGearMovementComponent;

UCLASS()
class ZELDAPHYSICS_API AZCGearBaseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZCGearBaseActor(const FObjectInitializer& ObjectInitializer);

public:

protected:
	virtual void PostInitializeComponents() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UZCGearMovementComponent> GearMovementComponent;
};
