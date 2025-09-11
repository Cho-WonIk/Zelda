// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Item/ZCItemActor.h"
#include "ZCShieldActor.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API AZCShieldActor : public AZCItemActor
{
	GENERATED_BODY()

public:
	AZCShieldActor();

	/*===========오버라이드 함수들===========*/
protected:
	virtual void PostInitializeComponents() override;

public:
	virtual void Initialize(FZCItemTable* NewItem) override;

	/*===========Getter===========*/
public:
	FORCEINLINE const struct FZCShieldTable* GetShieldInfo() const { return ShieldInfo; }

protected:
	struct FZCShieldTable* ShieldInfo;
};
