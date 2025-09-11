// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ZCGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:
	virtual void Init() override;
	virtual void Shutdown() override;
};
