// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "StateTreeTaskBase.h"
#include "AI/ZCAIControllerBase.h"
#include "Character/Monster/ZCMonsterCharacter.h"
#include "ZCStateTreeTaskBase.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "ZC스테이트 트리 태스크 베이스"))
class ZELDA_API UZCStateTreeTaskBase : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	AZCAIControllerBase* AIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	AZCMonsterCharacter* MonsterCharacter;
};
