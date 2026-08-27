
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameData/Enum/ZCActorType.h"
#include "GameplayTagContainer.h"
#include "GameData/PrimaryData/ZCActorPrimaryDataAsset.h"
#include "ZCActorTable.generated.h"

USTRUCT(BlueprintType)
struct FZCActorTable : public FTableRowBase
{
	GENERATED_BODY()

public:
	FZCActorTable() : ID(-1)
	{
		Type = EZCActortype::None;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "액터", meta = (DisplayName = "ID", DisplayPriority = 0))
	int32 ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "액터", meta = (DisplayName = "액터 분류", DisplayPriority = 1))
	EZCActortype Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "액터", meta = (DisplayName = "액터 물질", DisplayPriority = 10, GameplayTagFilter = "Material"))
	FGameplayTag MaterialTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "액터", meta = (DisplayName = "Primary애셋 데이터 ", DisplayPriority = 9))
	TSoftObjectPtr<UZCActorPrimaryDataAsset> Asset;

	bool operator==(const FZCActorTable& Other) const
	{
		return ID == Other.ID && Type == Other.Type && MaterialTag == Other.MaterialTag;
	}
};
