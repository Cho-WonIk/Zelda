// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZCPatrolSplineActor.generated.h"

UCLASS()
class ZELDA_API AZCPatrolSplineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZCPatrolSplineActor();

public:
	UFUNCTION(BlueprintCallable, Category = "Components")
	USplineComponent* GetSplineComponent() const { return SplineComponent; }

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void GenerateSplinePoints();

#if WITH_EDITOR
	void UpdateStartArrow();
#endif

protected:
	UPROPERTY(EditAnywhere, Category = "Spline Settings", meta = (ClampMin = "3", ClampMax = "64", UIMin = "3", UIMax = "64", DisplayName = "스플라인 포인트 수"))
	int32 NumPoints = 8;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
	TObjectPtr<class USplineComponent> SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
	TObjectPtr<class UArrowComponent> ArrowComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
	TObjectPtr<class USphereComponent> SphereComponent;
};
