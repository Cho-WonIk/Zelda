// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"
#include "Component/Reaction/Struct/ZCReactionEnum.h"
#include "GameData/Table/ChemistrySystemCharacterTable.h"
#include "Physics/ZCSurface.h"
#include "ZCStateComponent.generated.h"

USTRUCT(BlueprintType)
struct FCharacterElementState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ElementTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpreadingCount = -1;

	// 데미지 단발성 <-> 지속성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "데미지 지속성"))
	bool bIsDamageOnce = false;

	// 원소 틱 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 틱 데미지"))
	float ElementTickDamage = 0.0f;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "경직 여부"))
	bool bStiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "마찰력이 없어지는 지 여부"))
	bool bfriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force", meta = (DisplayName = "힘이 가해지는 지 여부"))
	bool bApplyForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force", meta = (DisplayName = "힘이 가해지는 힘의 크기"))
	float Force;

	static const FCharacterElementState Empty;

	[[nodiscard]] FORCEINLINE bool IsEmpty() const { return ElementTag == FGameplayTag::EmptyTag; }

	void Reset() { *this = Empty; }

	FCharacterElementState() : ElementTag(FGameplayTag::EmptyTag), Duration(-1.0f), SpreadingCount(-1), TimeRemaining(0.0f) {}
	FCharacterElementState(const FGameplayTag& InElementTag, float InDuration, int32 InPropagationCount) : ElementTag(InElementTag), Duration(InDuration), SpreadingCount(InPropagationCount), TimeRemaining(InDuration)
	{
		bIsDamageOnce = false;
		ElementTickDamage = 0.0f;
		SpreadingCount = -1;
		bStiffness = false;
		bfriction = false;

		bApplyForce = false;

		Force = 0.0f;
	}

	void InitFromOut(const FCharacterReactionOut& ReactionResult, const int32& InSpreadingCount)
	{
		ElementTag = ReactionResult.Tag;
		Duration = ReactionResult.Duration;
		bIsDamageOnce = ReactionResult.bIsDamageOnce;

		ElementTickDamage = ReactionResult.TickDamage;

		SpreadingCount = InSpreadingCount;

		TimeRemaining = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FCharacterArmorState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "방어력"))
	float ArmorState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "약점 속성"))
	TSet<FGameplayTag> WeakTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "강점 속성"))
	TSet<FGameplayTag> StrongTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "면역 속성"))
	TSet<FGameplayTag> ImmutTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 임계값"))
	TMap< FGameplayTag, float> Threshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 변화량"))
	TMap< FGameplayTag, float> ThresholdDelta;
};

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
struct FZCStagger
{
	GENERATED_BODY()

	// 피격 그로기 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "피격 그로기"))
	FZCStat DamageStagger;

	// CC기 그로기 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "CC기 그로기"))
	FCharacterElementState CCStagger;
};

class UZCWorldSubsystem;
class UZCNiagaraComponent;
struct FElementInfo;
struct FZCShape;
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
	// 접촉 데미지 전달 로직
	void NotifyHit(FVector& Location, FVector& Direction);

public:
	virtual float ApplyDamage(float DamageAmount, const FElementInfo& NewElementInfo, bool IsCritical);
	
	virtual void ApplyStagger(float AddStagger, const FElementInfo& NewElementInfo);

	bool IsDead() const { return Health.IsZero(); }

	void SetVFXComponent(UZCNiagaraComponent* VFXComp) { VFXComponent = VFXComp; }
	void SetOwnerCharacter(AZCCharacter* COwner) { OwnerCharacter = COwner; }

	bool IsWeakTag(FGameplayTag& Tag) { return Armor.WeakTag.Contains(Tag); }
	bool IsStrongTag(FGameplayTag& Tag) { return Armor.StrongTag.Contains(Tag); }
	bool IsImmutTag(FGameplayTag& Tag) { return Armor.ImmutTag.Contains(Tag); }

	bool IsDamageStaggerFull() { return Stagger.DamageStagger.IsFull(); }
	bool IsElementStaggerFull() { return !IsElementStaggerEmpty(); }

	bool IsElementStaggerEmpty() { return Stagger.CCStagger.IsEmpty(); }

	bool IsCanPlayElementStaggerMontage() { return IsElementStaggerFull() && Stagger.CCStagger.TimeRemaining == 0.0f; }

	void SetCharacterShape(struct FZCShape* NewShape) { CharacterShape = NewShape; }

protected:
	virtual float CalculDamage(float DamageAmount, const FGameplayTag& ElementTag, bool IsCritical);

	virtual void ApplyCCElement(const FCharacterReactionOut& ReactionResult, const int32& SpreadingCount);
	
	void ProcessElementSpreading();

private:
	void CCElementFX(bool bEnabled);

protected:
	// HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (DisplayName = "체력"))
	FZCStat Health;

	// 그로기 및 CC기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (DisplayName = "그로기 및 CC기"))
	FZCStagger Stagger;

	// 방어력, 몬스터의 경우 몬스터 속성이 적용됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (DisplayName = "방어력"))
	FCharacterArmorState Armor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "현재 원소 값"))
	TMap<FGameplayTag, float> CurrentThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "캐릭터 태그"))
	FGameplayTag CharacterTag;

	// CC기 : 스턴, 마찰, 틱데미지 유무
	/*
	* 모든 지속시간은 Duration에 따라 달라짐
	* 틱 데미지는 bIsTickableDamage에 따라 달라짐
	* 경직은 bStiffness으로 달라짐
	* 마찰력 유무는 bfriction
	* 힘 여부 bfriction
	* 
	*/

protected:
	class UZCWorldSubsystem* WorldSubsystem = nullptr;

	class UZCNiagaraComponent* VFXComponent;

	const FCharacterElementInstanceData* ElementData;

	struct FZCShape* CharacterShape;

	class AZCCharacter* OwnerCharacter = nullptr;

	// 마지막으로 충돌한 정보, 로컬값(Mesh 컴포넌트 기준 로컬지점과 법선 벡터)
	TArray<FZCSurfaceInfo> SufaceHitArray;
};
