

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ZCReactionEnum.h"
#include "ZCReactionStruct.generated.h"

USTRUCT(BlueprintType)
struct FReactionMontage
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (DisplayName = "히트 리액션 몽타주"))
	TObjectPtr<class UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (DisplayName = "히트 리액션 몽타주 배속"))
	float PlayRate = 1.0f;
};

USTRUCT(BlueprintType)
struct FZCReactionStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction", meta = (DisplayName = "히트 리액션 몽타주"))
	TMap<EZCHitDirection, FReactionMontage> HitReactionMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction", meta = (DisplayName = "히트 리액션 커브"))
	float DamageMultiplier = 1.0f;
};