// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Development/ZCDebug.h"
#if !UE_BUILD_SHIPPING
#include "AIController.h"
namespace Zelda::Debug::StateTree
{
	static bool bDrawDebugAll = false;
	static bool bDrawTask = false;
	static FAutoConsoleVariableRef CVar_DebugAll(Zelda::Debug::StateTree::all, bDrawDebugAll, TEXT("StateTree 디버깅 전체 On/Off"), ECVF_Default);
	static FAutoConsoleVariableRef CVar_DebugTask(Zelda::Debug::StateTree::task, bDrawTask, TEXT("StateTree 디버깅 Task On/Off"), ECVF_Default);
}
#endif

#include "ZCStateTreeInstanceBase.generated.h"


class AZCMonsterCharacter;
class AZCAIControllerBase;

USTRUCT()
struct FZCStandardInstanceData
{
	GENERATED_BODY()

	UPROPERTY(Transient, EditAnywhere, Category = "Context")
	AZCMonsterCharacter* MonsterCharacter;
	UPROPERTY(Transient, EditAnywhere, Category = "Context")
	AZCAIControllerBase* AIController;
};