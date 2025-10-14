// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Struct/Enum/ZCItemType.h"
#include "Physics/ZCCollision.h"
#include "ZCActor.generated.h"

class UZCNiagaraComponent;
class UZCMaterialStateComponent;

UCLASS()
class ZELDA_API AZCActor : public AActor
{
	GENERATED_BODY() 
	
public:	
	// Sets default values for this actor's properties
	AZCActor();

public:
	FORCEINLINE UZCMaterialStateComponent* GetMaterialStateComponent() const { return MaterialStateComponent; }
	FORCEINLINE UZCNiagaraComponent* GetZCNiagaraComponent() const { return NiagaraComponent; }

	virtual void PostInitializeComponents() override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<class UZCNiagaraComponent> NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material")
	TObjectPtr<class UZCMaterialStateComponent> MaterialStateComponent;

protected:
	// 액터가 원소와 상호작용하는지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zelda|Actor", meta = (AllowPrivateAccess = "true", DisplayName = "원소 상호작용 여부"))
	bool bCanElementalReaction = true;

protected:
};
