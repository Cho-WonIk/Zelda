
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "Physics/ZCShape.h"
#include "GameData/Enum/ZCChemistry.h"
#include "ChemistrySystemCharacterTable.generated.h"

// 데미지 적용 공식은 StateComponent에서 전담 여긴 단순히 BaseDamage만 설정

/*
고려사항

캐릭터별로 다른 효과가 적용될 수 있음
EX : 수속성 몬스터에게는 화염 데미지가 크리티컬로 들어감 등등

캐릭터에 적용될 결과들

데미지 : 단발 혹은 지속

경직 : 경직 여부

마찰력 : 마찰력 감소 여부

받는 힘 : 외력이 작용하는지 여부


[원소]
데미지
경직
마찰력
받는 힘

*/

USTRUCT(BlueprintType)
struct FCharacterMonsterTypeCDO : public FTableRowBase
{
	GENERATED_BODY()

	// 몬스터 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "캐릭터", GameplayTagFilter = "Monster"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "방어력"))
	float ArmorState;

	// 약점, 강점, 면역 속성에 따라 데미지 증폭
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "약점 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> WeakTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "강점 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> StrongTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "면역 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> ImmuntTag;

	// 원소별 임계치(예: 불, 물, 전기 등) 증감량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "임계값 증감량", GameplayTagFilter = "Element"))
	TMap<FGameplayTag, float> ThresholdDelta;

	// 원소별 임계치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "임계값", GameplayTagFilter = "Element"))
	TMap<FGameplayTag, float> Threshold;
};

USTRUCT(BlueprintType)
struct FCharacterElementCDO : public FTableRowBase
{
	GENERATED_BODY()

	// 원소 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소", GameplayTagFilter = "Element"))
	FGameplayTag Tag;

	// 확산 카운트 최대값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "확산될 수 있는 최대값"))
	int32 MaxSpreadingCount = 0;

	// 확산 방법
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "확산 방법", Bitmask, BitmaskEnum = ESpreadShapeType))
	uint8 SpreadType;

	// 확산 Shape, SpreadShape이 Element인 경우 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "확산 모양"))
	FZCShape SpreadShape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (DisplayName = "머티리얼"))
	TSoftObjectPtr<UMaterialInterface> ElementMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (DisplayName = "VFX 효과"))
	TSoftObjectPtr<UNiagaraSystem> VFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (DisplayName = "SFX 효과"))
	TSoftObjectPtr<USoundBase> SFX;

};

USTRUCT(BlueprintType)
struct FCharacterReactionOut : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "적용할 원소", GameplayTagFilter = "Element"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "지속시간"))
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (DisplayName = "데미지 단발성 여부"))
	bool bIsDamageOnce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (DisplayName = "초기 데미지"))
	float FirstDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (DisplayName = "틱 데미지"))
	float TickDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "경직 여부"))
	bool bStiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "마찰력이 없어지는 지 여부"))
	bool bfriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force", meta = (DisplayName = "힘이 가해지는 지 여부"))
	bool bApplyForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force", meta = (DisplayName = "힘이 가해지는 힘의 크기"))
	float Force;
};

USTRUCT(BlueprintType)
struct FCharacterReactionRuleRow : public FTableRowBase
{
	GENERATED_BODY()

	// 원소
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "적용될 원소", GameplayTagFilter = "Element"))
	FGameplayTag SourceElementTag;

	// 타겟(캐릭터의 태그)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "대상 캐릭터"))
	FGameplayTag CharacterTag;

	// 결과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "결과"))
	FCharacterReactionOut Outcome;
};

struct FCharacterReactionKey
{
	FGameplayTag SourceTag;

	FGameplayTag TargetTag;

	bool operator==(const FCharacterReactionKey& Rhs) const
	{
		return SourceTag == Rhs.SourceTag && TargetTag == Rhs.TargetTag;
	}

	friend uint32 GetTypeHash(const FCharacterReactionKey& K)
	{
		return HashCombine(GetTypeHash(K.SourceTag), GetTypeHash(K.TargetTag));
	}
};

USTRUCT(BlueprintType)
struct FCharacterElementInstanceData
{
	GENERATED_BODY()

	UPROPERTY(Transient) TObjectPtr<UNiagaraSystem> LoopVFX = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundBase> LoopSFX = nullptr;
	UPROPERTY(Transient) TObjectPtr<UMaterialInterface> LoopMaterial = nullptr;
	// 확산 카운트
	int32 MaxSpreadingCount = 0;

	// 확산 방법
	UPROPERTY(Transient) uint8 SpreadType;

	// 확산 Shape, SpreadShape이 Element인 경우 사용
	UPROPERTY(Transient) FZCShape SpreadShape;

	void InitFromCDO(const FCharacterElementCDO* InCDO)
	{
		LoopVFX = InCDO->VFX.Get();
		LoopSFX = InCDO->SFX.Get();
		LoopMaterial = InCDO->ElementMaterial.Get();

		MaxSpreadingCount = InCDO->MaxSpreadingCount;
		
		SpreadType = InCDO->SpreadType;
		SpreadShape = InCDO->SpreadShape;
	}
};

USTRUCT(BlueprintType)
struct FCharacterArmorTypeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(Transient) float ArmorState;
	UPROPERTY(Transient) TSet<FGameplayTag> WeakTag;

	UPROPERTY(Transient) TSet<FGameplayTag> StrongTag;

	UPROPERTY(Transient) TSet<FGameplayTag> ImmuntTag;

	UPROPERTY(Transient) TMap<FGameplayTag, float>  Threshold;
	UPROPERTY(Transient) TMap<FGameplayTag, float>  ThresholdDelta;

	// 몬스터 정보에서 초기화
	void InitFromMonsterCDO(const FCharacterMonsterTypeCDO* InCDO)
	{
		ArmorState = InCDO->ArmorState;
		
		WeakTag = InCDO->WeakTag;
		StrongTag = InCDO->StrongTag;
		ImmuntTag = InCDO->ImmuntTag;

		Threshold = InCDO->Threshold;
		ThresholdDelta = InCDO->ThresholdDelta;
	}
};