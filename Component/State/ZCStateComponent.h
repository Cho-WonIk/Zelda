// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"

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

DECLARE_MULTICAST_DELEGATE_OneParam(FZCStatChangedDelegate, const FZCStat&);

// Hit 이벤트 델리게이트
// HitCauser: Hit를 발생시킨 Actor
// HitResult: Hit의 결과 정보
// Enum As Byte: Hit의 강도 (예: 약한 공격, 강한 공격 등)
// UseCauseDeath: Hit이 사망을 유발하는지 여부
// UseHitCurserDirection: HitCauser의 방향을 사용할지 여부
DECLARE_MULTICAST_DELEGATE_FiveParams(FZCHitDelegate, const AActor* /*HitCauser*/, const FHitResult& /*HitResult*/, const uint8 /*Enum As Byte, InputHitStrength*/ , const bool /*UseCauseDeath*/, const bool /*UseHitCurserDirection*/);

DECLARE_MULTICAST_DELEGATE(FZCStatZeroDelegate);
DECLARE_MULTICAST_DELEGATE(FZCStatFullDelegate);

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

	FZCStatZeroDelegate OnHealthZero;
	FZCStatFullDelegate OnStaggerFull;

	FZCHitDelegate OnHit;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	//virtual void AddTag(const FGameplayTagContainer& Tags);
	//virtual void AddTag(const FGameplayTag& Tag);
	//virtual void RemoveTag(const FGameplayTagContainer& Tags);
	//virtual void RemoveTag(const FGameplayTag& Tag);
	virtual void ApplyDamage(float DamageAmount, const FGameplayTag& ElementTag, const FGameplayTag& DamageTypeTag, bool IsCritialBone, bool UseHitCuaserDirection, const FHitResult& HitResult, const AActor* HitCauser);
	//virtual void ApplyDamage(float DamageAmount, const FGameplayTag& DamageTypeTag, const FGameplayTag& ElementTag, bool IsCriticalBone);

	bool IsDead() const { return Health.IsZero(); }
	bool IsStaggerFull() const { return Stagger.IsFull(); }

protected:
	//// Health 관련 함수
	//void ApplyHeal(float HealAmount);

	//// 그로기 관련 함수
	//void AddStaggerGauge(float Value);

	//void UpdateStagger(float DeltaTime);

protected:
	// HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (DisplayName = "체력"))
	FZCStat Health;

	// 그로기 수치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (DisplayName = "그로기"))
	FZCStat Stagger;

	// 방어력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (DisplayName = "방어력"))
	float ArmorState = 0.0f;
	
	// 그로기 감소 시작 시간
	UPROPERTY(EditAnywhere, Category = "State|그로기", meta = (DisplayName = "그로기 감소 시작 시간"))
	float StaggerDecayAmount = 1.0f;

	// 그로기 감소량
	UPROPERTY(EditAnywhere, Category = "State|그로기", meta = (DisplayName = "그로기 감소량"))
	float StaggerDecayRate = 0.5f;

protected:
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = State)//, meta = (GameplayTagFilter = "Element"))
	FGameplayTag WeakElementTag;// = Element::None; // 약점 속성 태그

private:
	bool bCanDecreaseStagger = false;
	FTimerHandle StaggerDecayTimer;
};
