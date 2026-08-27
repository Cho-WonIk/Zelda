
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCDeviceEnum.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Ultrahand	UMETA(DisplayName = "울트라핸드"),
	Fuse		UMETA(DisplayName = "스크래빌드"),
	Ascend		UMETA(DisplayName = "트레루프"),
	Recall		UMETA(DisplayName = "리버레코"),
	Autobuild	UMETA(DisplayName = "블루프린트"),
	Max			UMETA(Hidden)
};
