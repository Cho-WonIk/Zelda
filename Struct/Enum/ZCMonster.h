#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ZCMonster.generated.h"

// 몬스터 역할 정의
UENUM(BlueprintType)
enum class EMonsterRole : uint8
{
	Scout			UMETA(DisplayName = "정찰병"),
	Infantry		UMETA(DisplayName = "보병"),
	Wather			UMETA(DisplayName = "감시병"),
	Archer			UMETA(DisplayName = "궁수"),
	Boss			UMETA(DisplayName = "보스"),
	None			UMETA(DisplayName = "없음"),
};

// 몬스터 종류
UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	Gobline		UMETA(DisplayName = "고블린"),
	Golem		UMETA(DisplayName = "골렘"),
	Lynel		UMETA(DisplayName = "라이넬"),
	None		UMETA(DisplayName = "없음"),
};