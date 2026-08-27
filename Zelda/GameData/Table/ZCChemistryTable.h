
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "GameData/Enum/ZCChemistry.h"
#include "GameData/Struct/ZCChemistryStruct.h"
#include "ZCChemistryTable.generated.h"

USTRUCT(BlueprintType)
struct FZCElementCDO : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 원소 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소", GameplayTagFilter = "Element"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 특성"))
	FZCElementValue Value;
};

USTRUCT(BlueprintType)
struct FZCMaterialCDO : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 물질 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "물질", GameplayTagFilter = "Material"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "물질 특성"))
	FZCSubstanceValue Value;
};

USTRUCT(BlueprintType)
struct FZCArmorChemistryCDO : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 방어구 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "방어구 태그"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "방어구 특성"))
	FZCArmorValue Value;
};

USTRUCT(BlueprintType)
struct FZCMonsterChemistryCDO : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 몬스터 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "몬스터", GameplayTagFilter = "Monster"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "몬스터 화학 특성"))
	FZCMonsterValue Value;
};

USTRUCT(BlueprintType)
struct FZCReactionRule : public FTableRowBase
{
	GENERATED_BODY()

	// 소스(보통 원소)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Source"))
	FGameplayTag SourceTag;

	// 타겟(재질이든 원소든 구분 없이 태그 하나)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Target"))
	FGameplayTag TargetTag;

	// 결과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Outcome"))
	FZCReactionOut Outcome;
};
