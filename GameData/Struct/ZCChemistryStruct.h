
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Physics/ZCSurface.h"
#include "ZCChemistryStruct.generated.h"

class AZCActor;
class UZCNiagaraComponent;

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


// 런타임 물질 상태 데이터
USTRUCT()
struct FMaterialRuntimeData
{
	GENERATED_BODY()

	// 소유자
	UPROPERTY()
	TWeakObjectPtr<AZCActor> OwnerActor;

	// VFX 제어
	UPROPERTY()
	TWeakObjectPtr<UZCNiagaraComponent> VFXComponent;

	// 물질 태그
	FGameplayTag MaterialTag;

	// CurrentElementState) ---
	FGameplayTag CurrentElementTag = FGameplayTag::EmptyTag;
	float Duration = -1.0f;
	int32 SpreadingCount = -1;
	bool bIsDamageOnce = false;
	float ElementTickDamage = 0.0f;

	// 임계치 누적 맵
	UPROPERTY()
	TMap<FGameplayTag, float> AccumulatedThresholds;

	// Surface hit 정보
	TArray<FZCSurfaceInfo> SurfaceHitArray;

	// 현재 내구도
	float CurrentDurability = 0.0f;

	bool IsActive() const { return CurrentElementTag != FGameplayTag::EmptyTag && (Duration > 0.0f || Duration == -1.0f); }

	bool IsEmpty() const { return !IsActive(); }

	void ResetState()
	{
		CurrentElementTag = FGameplayTag::EmptyTag;
		Duration = -1.0f;
		SpreadingCount = -1;
		bIsDamageOnce = false;
		ElementTickDamage = 0.0f;
		AccumulatedThresholds.Reset();
		SurfaceHitArray.Reset();
	}
};

// 핸들 관리용
struct FMaterialHandle
{
    int32 ID = INDEX_NONE;
    bool IsValid() const { return ID != INDEX_NONE; }

	bool operator==(const FMaterialHandle& Other) const { return ID == Other.ID; }
	bool operator!=(const FMaterialHandle& Other) const { return ID != Other.ID; }
};