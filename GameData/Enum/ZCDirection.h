#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCDirection.generated.h"

UENUM(BlueprintType)
enum class EZCDirection : uint8
{
	Forward			UMETA(DisplayName = "앞"),
	Backward		UMETA(DisplayName = "뒤"),
	Left			UMETA(DisplayName = "왼쪽"),
	Right			UMETA(DisplayName = "오른쪽"),
	ForwardLeft		UMETA(DisplayName = "앞왼쪽"),
	ForwardRight	UMETA(DisplayName = "앞오른쪽"),
	BackwardLeft	UMETA(DisplayName = "뒤왼쪽"),
	BackwardRight	UMETA(DisplayName = "뒤오른쪽"),
	None			UMETA(DisplayName = "None")
};
