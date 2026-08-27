#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCUIEvent.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EZCFaceButtonEvent : uint16
{
	None			= 0,
	PickUp			= 1 << 0,
	ClimbDown		= 1 << 1,
	Interact		= 1 << 2,
	Talk			= 1 << 3,
	Ambush			= 1 << 4,
	RideGear			= 1 << 5,
};
ENUM_CLASS_FLAGS(EZCFaceButtonEvent);

UENUM(BlueprintType)
enum class EZCHUDMode : uint8
{
	Normal,
	Skill,
	Gear
};
