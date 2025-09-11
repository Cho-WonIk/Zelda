// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "ZCNiagaraComponent.generated.h"

struct FGameplayTag;

UCLASS()
class ZELDA_API UZCNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UZCNiagaraComponent();

public:
	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "엘리먼트 상호작용"))
	void PlayElementReactionVFX(FGameplayTag& PlayTag);
};
