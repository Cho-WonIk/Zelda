// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "ZCEQSComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCEQSComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCEQSComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category = "EQS")
	void RunEQSQuery();

	UFUNCTION(BlueprintCallable, Category = "EQS")
	FVector GetBestLocation() const;

	UFUNCTION(BlueprintCallable, Category = "EQS")
	AActor* GetBestActor() const;

protected:
	UPROPERTY(EditAnywhere, Category = "EQS")
	TObjectPtr<class UEnvQuery> EQSQuery;

	UPROPERTY(EditAnywhere, Category = "EQS")
	TEnumAsByte<EEnvQueryRunMode::Type> RunMode = EEnvQueryRunMode::RandomBest25Pct;

private:
	UPROPERTY()
	TObjectPtr<class UEnvQueryInstanceBlueprintWrapper> QueryResultWrapper;

	UPROPERTY()
	TArray<FVector> CachedLocations;

	UPROPERTY()
	TArray<AActor*> CachedActors;

	// 콜백 핸들러
	UFUNCTION()
	void OnQueryFinished(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);
};
