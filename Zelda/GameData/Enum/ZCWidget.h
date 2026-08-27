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

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFaceButtonType : uint8
{
	None	= 0,
	AButton = 1 << 0,
	BButton = 1 << 1,
	XButton = 1 << 2,
	YButton = 1 << 3
}; 
ENUM_CLASS_FLAGS(EFaceButtonType);
