
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCReactionEnum.generated.h"

UENUM(BlueprintType)
enum class EZCHitDirection : uint8
{
	Front	UMETA(DisplayName = "앞"),
	Back	UMETA(DisplayName = "뒤"),
	Left	UMETA(DisplayName = "왼쪽"),
	Right	UMETA(DisplayName = "오른쪽"),
	Top		UMETA(DisplayName = "위"),
	Bottom	UMETA(DisplayName = "아래")
};

UENUM(BlueprintType)
enum class EHitStrength : uint8
{
	Light		UMETA(DisplayName = "가벼운 충격"),
	Medium		UMETA(DisplayName = "중간 충격"),
	Heavy		UMETA(DisplayName = "강한 충격"),
	Explosion	UMETA(DisplayName = "폭발 충격"),
};

UENUM(BlueprintType)
enum class EHitPosition : uint8
{
	Head			UMETA(DisplayName = "머리"),
	Chest			UMETA(DisplayName = "가슴"),
	Abdomen			UMETA(DisplayName = "복부"),
	Pelvis			UMETA(DisplayName = "골반"),
	LeftShoulder	UMETA(DisplayName = "왼쪽 어깨"),
	RightShoulder	UMETA(DisplayName = "오른쪽 어깨"),
	LeftArm			UMETA(DisplayName = "왼쪽 팔"),
	RightArm		UMETA(DisplayName = "오른쪽 팔"),
	LeftThigh		UMETA(DisplayName = "왼쪽 허벅지"),
	RightThigh		UMETA(DisplayName = "오른쪽 허벅지"),
	LeftCalf		UMETA(DisplayName = "왼쪽 종아리"),
	RightCalf		UMETA(DisplayName = "오른쪽 종아리"),
	Other			UMETA(DisplayName = "기타")
};