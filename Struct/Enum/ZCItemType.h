#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCItemType.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None        UMETA(DisplayName = "아이템 없음"),
	Normal      UMETA(DisplayName = "일반 아이템"),	// 일반 아이템
    Weapon	    UMETA(DisplayName = "무기"),
    Shield      UMETA(DisplayName = "방패"),
	Armor	    UMETA(DisplayName = "갑옷"),
    Consumable  UMETA(DisplayName = "소비템")
    // 요리
};

UENUM(BlueprintType)
enum class Rarity : uint8
{
    Common		UMETA(DisplayName = "기본 아이템"),      // 기본 아이템
    Uncommon	UMETA(DisplayName = "드문 아이템"),		// 약간 드문 아이템
    Rare		UMETA(DisplayName = "희귀 아이템"),		// 희귀 아이템
    Epic		UMETA(DisplayName = "에픽 아이템"),		// 에픽 아이템
    Legendary	UMETA(DisplayName = "전설 아이템"),		// 전설 아이템
    Mythic		UMETA(DisplayName = "유일 아이템")		// 유일 아이템(인 게임에서 1개만 존재)
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None			UMETA(DisplayName = "무기 없음"),
    OneHandSword	UMETA(DisplayName = "한손검"),
    TwoHandSword	UMETA(DisplayName = "양손검"),
    Spear			UMETA(DisplayName = "창"),
    Bow				UMETA(DisplayName = "활")
};

UENUM(BlueprintType)
enum class EShieldType : uint8
{
	None			UMETA(DisplayName = "방패 없음"),
	Shield		    UMETA(DisplayName = "방패"),
};