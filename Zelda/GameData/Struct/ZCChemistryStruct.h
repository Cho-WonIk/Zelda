
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "Physics/ZCSurface.h"
#include "Physics/ZCShape.h"
#include "ZCChemistryStruct.generated.h"

class AZCActor;
class UZCNiagaraComponent;

USTRUCT(BlueprintType)
struct FZCReactionOut
{
	GENERATED_BODY()

public:
	// 부착할 원소 엘리먼트, 없을 수 있음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (GameplayTagFilter = "Element"))
	FGameplayTag ReactionElementTag;

	// 원소 지속시간, -1.0f인경우 무한이 지속
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "지속시간", ToolTip = "원소 지속 시간"))
	float Duration = 0.0f;

	// 데미지 단발성 <-> 지속성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "데미지 지속성"))
	bool bIsDamageOnce = true;

	// 액터가 입는 원소 초기 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 초기데미지"))
	float FirstDamage = 0.0f;

	// 액터가 입는 틱 데미지 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 틱 데미지 간격"))
	float TickInterval;

	// 액터가 입는 원소 틱 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 틱 데미지"))
	float TickDamage;

};

struct FZCReactionKey
{
	FGameplayTag SourceTag;

	FGameplayTag TargetTag;

	bool operator==(const FZCReactionKey& Rhs) const { return SourceTag == Rhs.SourceTag && TargetTag == Rhs.TargetTag; }

	friend uint32 GetTypeHash(const FZCReactionKey& K) { return HashCombine(GetTypeHash(K.SourceTag), GetTypeHash(K.TargetTag)); }
};

USTRUCT(BlueprintType)
struct FZCEffectVisuals
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "VFX"))
	TSoftObjectPtr<UNiagaraSystem> VFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "SFX"))
	TSoftObjectPtr<USoundBase> SFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "머티리얼"))
	TSoftObjectPtr<UMaterialInterface> ElementMaterial;
};

// 메모리에 로드될 애셋 객체
struct FZCLoadedVisual
{
public:
	TStrongObjectPtr<UNiagaraSystem> VFX;
	TStrongObjectPtr<USoundBase> SFX;
	TStrongObjectPtr<UMaterialInterface> Material;

	bool IsValid() const { return VFX.IsValid() || SFX.IsValid() || Material.IsValid(); }

	void Reset()
	{
		VFX.Reset();
		SFX.Reset();
		Material.Reset();
	}
};

// 원소 FX 로드용 객체, 오브젝트용과 캐릭터용 FX전부 소유
struct FZCLoadedElementAssetData
{
	// 정적 오브젝트용 FX 세트
	FZCLoadedVisual ObjectVisuals;

	// 캐릭터용 FX 세트
	FZCLoadedVisual CharacterVisuals;

	void Reset()
	{
		ObjectVisuals.Reset();
		CharacterVisuals.Reset();
	}
};

USTRUCT(BlueprintType)
struct FZCElementValue
{
	GENERATED_BODY()

public:
	// 확산 카운트 최대값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "확산될 수 있는 최대값"))
	int32 MaxSpreadingCount = 0;

	// 확산 방법
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "확산 방법", Bitmask, BitmaskEnum = ESpreadShapeType))
	uint8 SpreadType;

	// 확산 Shape, SpreadShape이 Element인 경우 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "확산 모양"))
	FZCShape SpreadShape;

	// 정적 오브젝트에 적용될 FX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (DisplayName = "정적 오브젝트용 FX"))
	FZCEffectVisuals ObjectVisuals;

	// 캐릭터에 적용될 FX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (DisplayName = "캐릭터용 FX"))
	FZCEffectVisuals CharacterVisuals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (DisplayName = "마찰력이 없어지는 지 여부"))
	bool bfriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (DisplayName = "힘이 가해지는 지 여부"))
	bool bApplyForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (DisplayName = "힘이 가해지는 힘의 크기"))
	float Force;
};

USTRUCT(BlueprintType)
struct FZCSubstanceValue
{
	GENERATED_BODY()

public:
	// 원소별 임계치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "임계값", GameplayTagFilter = "Element"))
	TMap<FGameplayTag, float> Threshold;
};

USTRUCT(BlueprintType)
struct FZCArmorValue : public FZCSubstanceValue
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "약점 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> WeakTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "강점 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> StrongTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "면역 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> ImmuntTag;
};

USTRUCT(BlueprintType)
struct FZCMonsterValue : public FZCSubstanceValue
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "약점 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> WeakTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "강점 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> StrongTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "면역 속성", GameplayTagFilter = "Element"))
	TSet<FGameplayTag> ImmuntTag;
};

USTRUCT(BlueprintType)
struct FElementInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소"))
    FGameplayTag ElementTag = FGameplayTag::EmptyTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "공격자 원소의 지속시간"))
    float Duration = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "공격자 원소의 확산 카운트"))
    int32 SpreadCount = -1;

    FElementInfo() {}

    FElementInfo(FGameplayTag InElementTag) : ElementTag(InElementTag) {}

    FElementInfo(FGameplayTag InElementTag, float InDuration, int32 InSpreadCount)
    {
        ElementTag = InElementTag;
        Duration = InDuration;
        SpreadCount = InSpreadCount;
    }

    [[nodiscard]] const bool IsEmpty() const { return ElementTag == FGameplayTag::EmptyTag; }
    [[nodiscard]] const bool IsNewElement() const { return Duration == -1.0f && SpreadCount == -1; }
};

// 핸들 관리용
struct FZCChemistryHandle
{
	int32 ID = INDEX_NONE;
	bool IsValid() const { return ID != INDEX_NONE; }

	bool operator==(const FZCChemistryHandle& Other) const { return ID == Other.ID; }
	bool operator!=(const FZCChemistryHandle& Other) const { return ID != Other.ID; }
};

// 런타임 오브젝트 상태 데이터
struct FZCRuntimeActorChemistryState
{
public:
	FZCRuntimeActorChemistryState()
	{
		SurfaceHitArray.Reserve(10);
		Reset();
	}

	TWeakObjectPtr<AZCActor> Owner;
	TWeakObjectPtr<UZCNiagaraComponent> VFXComponent;

	FGameplayTag MaterialTag;

	// 물질 정보 포인터 캐싱
	const FZCSubstanceValue* MaterialData = nullptr;

	FGameplayTag CurrentElementTag = FGameplayTag::EmptyTag;
	// 원소 지속시간, -1.0f인경우 무한이 지속
	float Duration = 0.0f;
	// 확산 카운트
	int32 SpreadCount = -1;
	// 확산 쿨타임 타이머
	float SpreadTimer = 0.0f;

	// 데미지 단발성 <-> 지속성
	bool bIsDamageOnce = true;
	// 액터가 입는 틱 데미지 간격(0.0은 기본 Tick간격)
	float TickInterval;
	// 액터가 입는 원소 틱 데미지
	float TickDamage;
	// 데미지 틱 처리를 위한 타이머
	float DamageTimer = 0.0f;

	// 표면 정보
	TArray<FZCSurfaceInfo> SurfaceHitArray;

	// 원소별 누적값
	TMap<FGameplayTag, float> AccumulatedThresholds;

	void RandomizeSpreadTimer(float BaseInterval) { SpreadTimer = BaseInterval + FMath::FRandRange(0.0f, 0.05f); }

	bool IsActive() const { return CurrentElementTag != FGameplayTag::EmptyTag; }
	bool IsDeactive() const { return !IsActive(); }

	bool IsValid() const { return Owner.IsValid() && VFXComponent.IsValid(); }

	void ResetElementState()
	{
		CurrentElementTag = FGameplayTag::EmptyTag;
		Duration = -1.0f;
		SpreadCount = -1;
		SpreadTimer = 0.0f;

		bIsDamageOnce = false;
		TickInterval = 0.0f;
		TickDamage = 0.0f;
		DamageTimer = 0.0f; // 타이머 초기화

		SurfaceHitArray.Reset();
		AccumulatedThresholds.Reset();
	}

	void Reset()
	{
		Owner = nullptr;
		VFXComponent = nullptr;
		MaterialTag = FGameplayTag::EmptyTag;
		MaterialData = nullptr;

		ResetElementState();
	}
};
