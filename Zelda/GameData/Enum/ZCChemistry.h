#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCChemistry.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESpreadShapeType : uint8
{
	None	= 0,
	Element = 1 << 0,
	Object	= 1 << 1
};
ENUM_CLASS_FLAGS(ESpreadShapeType);
