// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "GameplayTagContainer.h"
#include "ZCDamage.generated.h"

// 공격의 원소 속성 태그를 정의
UCLASS(const, Blueprintable, BlueprintType)
class ZELDA_API UZCDamageType : public UDamageType
{
	GENERATED_UCLASS_BODY()
	
public:
	/** 공격에 적용된 공격 속성(참쇄, 둔격, 타격 등등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage, meta = (GameplayTagFilter = "DamageType"))
	FGameplayTag DamageTypeTag;
	
};

USTRUCT()
struct FZCDamageEvent : public FDamageEvent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element, meta = (GameplayTagFilter = "Element"))
	FGameplayTag ElementTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element)
	float ElementDuration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element)
	int32 ElementSpreadingCount = -1;

	FZCDamageEvent() : FDamageEvent() {}

	FZCDamageEvent(TSubclassOf<UDamageType> InDamageTypeClass, const FGameplayTag& InElement)
		: FDamageEvent(InDamageTypeClass)
		, ElementTag(InElement)
	{}

	FZCDamageEvent(TSubclassOf<UDamageType> InDamageTypeClass, const FGameplayTag& InElement, float InElementDuration, int32 InElementSpreadingCount)
		: FDamageEvent(InDamageTypeClass)
		, ElementTag(InElement)
		, ElementDuration(InElementDuration)
		, ElementSpreadingCount(InElementSpreadingCount)
	{}

	// 클래스 ID, 101로 시작
	static const int32 ClassID = 101;

	virtual int32 GetTypeID() const { return FZCDamageEvent::ClassID; }
	virtual bool IsOfType(int32 InID) const { return FZCDamageEvent::ClassID == InID; };
};

USTRUCT()
struct FZCPointDamageEvent : public FPointDamageEvent
{
	GENERATED_BODY()
public:
	/** 원소 속성 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element, meta = (GameplayTagFilter = "Element"))
	FGameplayTag ElementTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element)
	float ElementDuration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element)
	int32 ElementSpreadingCount = -1;

	FZCPointDamageEvent() : FPointDamageEvent() , ElementTag(FGameplayTag::EmptyTag) {}

	FZCPointDamageEvent(float InDamage, const FHitResult& InHitInfo, const FVector& InShotDirection, TSubclassOf<UDamageType> InDamageTypeClass, const FGameplayTag& InElementTag)
		: FPointDamageEvent(InDamage, InHitInfo, InShotDirection, InDamageTypeClass)
		, ElementTag(InElementTag)
	{}

	FZCPointDamageEvent(float InDamage, const FHitResult& InHitInfo, const FVector& InShotDirection, TSubclassOf<UDamageType> InDamageTypeClass, const FGameplayTag& InElementTag, float InElementDuration, int32 InElementSpreadingCount)
		: FPointDamageEvent(InDamage, InHitInfo, InShotDirection, InDamageTypeClass)
		, ElementTag(InElementTag)
		, ElementDuration(InElementDuration)
		, ElementSpreadingCount(InElementSpreadingCount)
	{}

	// 클래스 ID
	static const int32 ClassID = 102;

	virtual int32 GetTypeID() const { return FZCPointDamageEvent::ClassID; }
	virtual bool IsOfType(int32 InID) const { return FZCPointDamageEvent::ClassID == InID; };
};

USTRUCT()
struct FZCRadialDamageEvent : public FRadialDamageEvent
{
	GENERATED_BODY()
public:
	/** 원소 속성 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element, meta = (GameplayTagFilter = "Element"))
	FGameplayTag ElementTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element)
	float ElementDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Element)
	int32 ElementSpreadingCount = -1;

	FZCRadialDamageEvent() : FRadialDamageEvent(), ElementTag(FGameplayTag::EmptyTag) {}

	FZCRadialDamageEvent(const FRadialDamageParams& InParams, const FVector& InOrigin, TSubclassOf<UDamageType> InDamageTypeClass, const FGameplayTag& InElementTag)
	{
		Params = InParams;
		Origin = InOrigin;
		DamageTypeClass = InDamageTypeClass;
		ElementTag = InElementTag;
	}

	FZCRadialDamageEvent(const FRadialDamageParams& InParams, const FVector& InOrigin, TSubclassOf<UDamageType> InDamageTypeClass, const FGameplayTag& InElementTag, float InElementDuration, int32 InElementSpreadingCount)
	{
		Params = InParams;
		Origin = InOrigin;
		DamageTypeClass = InDamageTypeClass;
		ElementTag = InElementTag;
		ElementDuration = InElementDuration;
		ElementSpreadingCount = InElementSpreadingCount;
	}

	// 클래스 ID
	static const int32 ClassID = 103;
	virtual int32 GetTypeID() const { return FZCRadialDamageEvent::ClassID; }
	virtual bool IsOfType(int32 InID) const { return FZCRadialDamageEvent::ClassID == InID; };
};