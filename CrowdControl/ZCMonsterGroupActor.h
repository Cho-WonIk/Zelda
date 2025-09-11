// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZCMonsterGroupActor.generated.h"

class UZCEQSComponent;

UCLASS()
class ZELDA_API AZCMonsterGroupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZCMonsterGroupActor();

public:
	UFUNCTION(BlueprintCallable, Category = "Component")
	FORCEINLINE USphereComponent* GetMonsterArea() const { return MonsterArea; }

	UFUNCTION(BlueprintCallable, Category = "Component")
	FORCEINLINE UZCEQSComponent* GetEQSComponent() const { return EQSComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// 액터가 배치되거나 수정되었을 때 호출됨
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Radius", meta = (DisplayName = "몬스터 영역 반경"))
	float MonsterRadius = 300.0f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class USphereComponent> MonsterArea;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UZCEQSComponent> EQSComponent;
};
