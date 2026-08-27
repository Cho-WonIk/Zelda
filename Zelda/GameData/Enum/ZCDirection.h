#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCDirection.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EZCDirection : uint8
{
	None			= 0				UMETA(DisplayName = "None"),
	Forward			= 1 << 0		UMETA(DisplayName = "앞"),
	Backward		= 1 << 1		UMETA(DisplayName = "뒤"),
	Left			= 1 << 2		UMETA(DisplayName = "왼쪽"),
	Right			= 1 << 3		UMETA(DisplayName = "오른쪽"),
	Up				= 1 << 4		UMETA(DisplayName = "위"),
	Down			= 1 << 5		UMETA(DisplayName = "아래"),
	Max				= 1 << 7		UMETA(Hidden)
};
ENUM_CLASS_FLAGS(EZCDirection);
