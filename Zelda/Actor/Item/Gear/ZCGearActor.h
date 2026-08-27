// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ZCStaticActor.h"
#include "ZCGearActor.generated.h"

class UZCGearMovementComponent;

UCLASS()
class ZELDA_API AZCGearActor : public AZCStaticActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AZCGearActor(const FObjectInitializer& ObjectInitializer);

	UZCGearMovementComponent* GetGearMovementComponent() const { return GearMovementComponent; }

protected:
	virtual void PostInitializeComponents() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UZCGearMovementComponent> GearMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> SphereInteractionArea;
};

UCLASS()
class ZELDA_API AZCGearFanActor : public AZCGearActor
{
	GENERATED_BODY()

public:
	AZCGearFanActor(const FObjectInitializer& ObjectInitializer);
};

UCLASS()
class ZELDA_API AZCGearBalloonActor : public AZCGearActor
{
	GENERATED_BODY()

public:
	AZCGearBalloonActor(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void GetElement(const FGameplayTag& ElementTag) override;

	void OnFireTimeOut();

private:
	FTimerHandle BalloonPopTimerHandle;
};
