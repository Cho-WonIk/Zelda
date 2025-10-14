
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "ChemistrySystemTable.generated.h"

/*================= 원소와 물체간의 상호작용 =================*/

USTRUCT(BlueprintType)
struct FMaterialCDO : public FTableRowBase
{
	GENERATED_BODY()

	// 물질 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "물질", GameplayTagFilter = "Material"))
	FGameplayTag Tag;

	// 원소별 임계치(예: 불, 물, 전기 등) 증감량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "임계값 증감량", GameplayTagFilter = "Element"))
	TMap<FGameplayTag, float> ThresholdDelta;

	// 원소별 임계치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "임계값", GameplayTagFilter = "Element"))
	TMap<FGameplayTag, float> Threshold;

	// 물질의 내구도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "내구도"))
	float Durability;
};

USTRUCT(BlueprintType)
struct FElementCDO : public FTableRowBase
{
	GENERATED_BODY()

	// 원소 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소", GameplayTagFilter = "Element"))
	FGameplayTag Tag;

	// 확산 카운트 최대값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "확산될 수 있는 최대값"))
	int32 MaxSpreadingCount = 0;

	// 확산 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "확산 범위"))
	float SpreadingRange = 0.0f;

	// 원소가 주변에 확산될 때 주는 데미지, 0.0일 경우 데미지 전달은 없고 확산만 일어남
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 데미지"))
	float Damage = 0.0f;

	/*==== FX ====*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (DisplayName = "지속되는 VFX"))
	TSoftObjectPtr<UNiagaraSystem> LoopVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX", meta = (DisplayName = "지속되는 SFX"))
	TSoftObjectPtr<USoundBase> LoopSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (DisplayName = "시작 VFX"))
	TSoftObjectPtr<UNiagaraSystem> StartVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX", meta = (DisplayName = "시작 SFX"))
	TSoftObjectPtr<USoundBase> StartSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (DisplayName = "끝 VFX"))
	TSoftObjectPtr<UNiagaraSystem> EndVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX", meta = (DisplayName = "끝 SFX"))
	TSoftObjectPtr<USoundBase> EndSFX;
};

USTRUCT(BlueprintType)
struct FReactionOut
{
	GENERATED_BODY()

	// 부착할 원소 엘리먼트, 없을 수 있음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (GameplayTagFilter = "Element"))
	FGameplayTag NewElementTag;

	// 데미지 단발성 <-> 지속성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "데미지 지속성"))
	bool bIsDamageOnce = true;

	// 액터가 입는 원소 초기 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 초기데미지"))
	float ElementFirstDamage = 0.0f;

	// 액터가 입는 원소 틱 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 틱 데미지"))
	float ElementTickDamage;

	// 원소 지속시간, -1.0f인경우 무한이 지속
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "지속시간", ToolTip = "원소 지속 시간"))
	float Duration = 0.0f;
};

USTRUCT(BlueprintType)
struct FReactionRuleRow : public FTableRowBase
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
	FReactionOut Outcome;
};

struct FReactionKey
{
	FGameplayTag SourceTag;

	FGameplayTag TargetTag;

	bool operator==(const FReactionKey& Rhs) const
	{
		return SourceTag == Rhs.SourceTag && TargetTag == Rhs.TargetTag;
	}

	friend uint32 GetTypeHash(const FReactionKey& K)
	{
		return HashCombine(GetTypeHash(K.SourceTag), GetTypeHash(K.TargetTag));
	}
};

USTRUCT()
struct FElementInstanceData
{
	GENERATED_BODY()

	UPROPERTY(Transient) TObjectPtr<UNiagaraSystem> StartVFX = nullptr;
	UPROPERTY(Transient) TObjectPtr<UNiagaraSystem> LoopVFX = nullptr;
	UPROPERTY(Transient) TObjectPtr<UNiagaraSystem> EndVFX = nullptr;

	UPROPERTY(Transient) TObjectPtr<USoundBase> StartSFX = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundBase> LoopSFX = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundBase> EndSFX = nullptr;

	// 확산 카운트
	int32 MaxSpreadingCount = 0;

	// 확산 범위
	float SpreadingRange = 0;

	// 주변에 주는 데미지
	float Damage = 0.0f;

	void InitFromCDO(const FElementCDO* InCDO)
	{
		StartVFX = InCDO->StartVFX.Get();
		LoopVFX = InCDO->LoopVFX.Get();
		EndVFX = InCDO->EndVFX.Get();

		StartSFX = InCDO->StartSFX.Get();
		LoopSFX = InCDO->LoopSFX.Get();
		EndSFX = InCDO->EndSFX.Get();

		MaxSpreadingCount = InCDO->MaxSpreadingCount;
		SpreadingRange = InCDO->SpreadingRange;

		Damage = InCDO->Damage;
	}
};

USTRUCT()
struct FMaterialInstanceData
{
	GENERATED_BODY()

	// 원소별 임계치(예: 불, 물, 전기 등) 증감량
	UPROPERTY(Transient) TMap<FGameplayTag, float> ThresholdDelta;

	// 원소별 임계치
	UPROPERTY(Transient) TMap<FGameplayTag, float> Threshold;

	// 물질의 내구도
	UPROPERTY(Transient) float Durability;

	void InitFromCDO(const FMaterialCDO* InCDO)
	{
		Threshold = InCDO->Threshold;
		ThresholdDelta = InCDO->ThresholdDelta;
		Durability = InCDO->Durability;
	}
};