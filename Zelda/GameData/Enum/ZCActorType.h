#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCActorType.generated.h"

UENUM(BlueprintType)
enum class EZCActortype : uint8
{
	None			UMETA(Hidden),
	Environment		UMETA(DisplayName = "환경"),
	Interaction		UMETA(DisplayName = "상호작용가능"),
	Item			UMETA(DisplayName = "아이템")
};