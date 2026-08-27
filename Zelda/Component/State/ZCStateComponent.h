// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"
#include "Component/Reaction/Struct/ZCReactionEnum.h"

#include "GameData/Table/ZCChemistryTable.h"
#include "Physics/ZCSurface.h"
#include "ZCStateComponent.generated.h"

USTRUCT(BlueprintType)
struct FZCStat
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stat")
	float Current;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Max;

	FZCStat()				: Current(0.0f), Max(0.0f) {}
	FZCStat(float InMax)	: Current(InMax), Max(InMax) {}
	FZCStat(float InCurrent, float InMax) : Current(InCurrent), Max(InMax) {}

	// In = Max = Current
	void Set(float In) { Current = In; Max = In; }
	// In1 = Max, In2 = Current
	void Set(float In1, float In2) { Max = In1; Current = FMath::Clamp(In2, 0.0f, In1); }

	void SetMax()	{ Current = Max; }
	void SetZero()	{ Current = 0.0f; }

	bool IsZero() const { return Current <= 0.0f; }
	bool IsFull() const { return Current >= Max; }

	void Add(float Value) { Current = FMath::Clamp(Current + Value, 0.0f, Max); }
	void Sub(float Value) { Current = FMath::Clamp(Current - Value, 0.0f, Max); }

	float GetNormalized() const { return (Max > 0.0f) ? Current / Max : 0.0f; }
};

USTRUCT(BlueprintType)
struct FZCCharacterElementStat
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chemistry", meta = (DisplayName = "현재 적용된 원소"))
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

	// CC기 지속시간을 위한 타이머
	float TimeRemaining = 0.0f;

	// 원소별 누적된 값
	TMap<FGameplayTag, float> AccumulatedThresholds;
	// 표면 정보
	TArray<FZCSurfaceInfo> SurfaceHitArray;

	bool IsActive() const { return CurrentElementTag != FGameplayTag::EmptyTag && (Duration > 0.0f || Duration == -1.0f); }
	bool IsDeactive() const { return !IsActive(); }

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

		AccumulatedThresholds.Reset();
		SurfaceHitArray.Reset();
	}

	void Reset()
	{
		ResetElementState();
	}
};

USTRUCT(BlueprintType)
struct FZCStagger
{
	GENERATED_BODY()

	// 피격 그로기 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "피격 그로기"))
	FZCStat DamageStagger;

	// 원소 누적치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 그로기"))
	FZCCharacterElementStat ElementStagger;
};

// 플레이어의 경우 방어구의 값을 받아옴
// 몬스터의 경우 몬스터 테이블에 정의된 값을 받아옴
USTRUCT(BlueprintType)
struct FZCCharacterArmorStat
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "방어력 수치"))
	float ArmorState = 0.0f;

	const TMap<FGameplayTag, float>* Threshold;
	const TSet<FGameplayTag>* WeakTag;
	const TSet<FGameplayTag>* StrongTag;
	const TSet<FGameplayTag>* ImmuntTag;
};


class UZCWorldSubsystem;
class UZCNiagaraComponent;
struct FElementInfo;
struct FZCSurfaceInfo;
class AZCCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FZCStatChangedDelegate, const FZCStat&);

DECLARE_MULTICAST_DELEGATE(FZCStatDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCStateComponent();

	// 델리게이트
public:
	FZCStatChangedDelegate OnHealthChanged;
	FZCStatDelegate OnHealthZero;
	FZCStatDelegate OnStaggerFull;

	//FZCHitDelegate OnHit;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void InitializeComponent() override;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void SetVFXComponent(UZCNiagaraComponent* VFXComp) { VFXComponent = VFXComp; }
	void SetOwnerCharacter(AZCCharacter* COwner) { OwnerCharacter = COwner; }

	// 접촉 데미지 전달 로직
	void NotifyHit(FVector& Location, FVector& Direction);

	// 데미지 및 상태 적용
	virtual float ApplyDamage(float DamageAmount, const FElementInfo& NewElementInfo, bool IsCritical);
	virtual void ApplyStagger(float AddStagger, const FElementInfo& NewElementInfo);

	// 상태 확인
	bool IsDead() const { return Health.IsZero(); }

	// 약점 속성
	bool IsWeak(const FGameplayTag& Tag) { return Armor.WeakTag->Contains(Tag); }
	// 강점 속성
	bool IsStrong(const FGameplayTag& Tag) { return Armor.StrongTag->Contains(Tag); }
	// 면역 속성
	bool IsImmunt(const FGameplayTag& Tag) { return Armor.ImmuntTag->Contains(Tag); }

	bool IsDamageStaggerFull() { return Stagger.DamageStagger.IsFull(); }

	bool IsElementStaggerActive() { return Stagger.ElementStagger.IsActive(); }
	bool IsElementStaggerDeactive() { return Stagger.ElementStagger.IsDeactive(); }

	bool IsCanPlayElementStaggerMontage() { return IsElementStaggerActive() && Stagger.ElementStagger.TimeRemaining == 0.0f; }

protected:
	// 하위 클래스에서 FZCCharacterArmorStat에 할당
	virtual void SetArmor() {}

	// 데미지 계산 (속성 상성 반영)
	virtual float CalculDamage(float DamageAmount, const FGameplayTag& ElementTag, bool IsCritical);

	// 화학 반응 결과 적용
	virtual void ApplyCCElement(const FZCReactionOut& ReactionResult, const int32& SpreadCount);
	
	// 주변 원소 전파
	void ProcessElementSpreading(float DeltaTime);

private:
	// 원소 FX적용
	void CCElementFX(bool bEnabled);

protected:
	// HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (DisplayName = "체력"))
	FZCStat Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "캐릭터 태그"))
	FGameplayTag CharacterTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "그로기"))
	FZCStagger Stagger;

	FZCCharacterArmorStat Armor;

protected:
	class UZCWorldSubsystem* WorldSubsystem = nullptr;
	class UZCNiagaraComponent* VFXComponent;
	class AZCCharacter* OwnerCharacter = nullptr;

	float SpreadInterval = 0.0f;
};
