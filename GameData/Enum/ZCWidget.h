#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCWidget.generated.h"

UENUM(BlueprintType)
enum class EWheelSlotType : uint8
{
	Procedural,
	CustomImage
};

UENUM(BlueprintType)
enum class EWheelPosition : uint8
{
	Center,
	Left,
	Right
};