// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Item/ZCItemActor.h"
#include "ZCWeaponActor.generated.h"

UENUM(BlueprintType)
enum EAttackType : uint8
{
	Normal,
	Charging,
	AttackCharge
};

class UZCHitTraceComponent;

UCLASS(Blueprintable)
class ZELDA_API AZCWeaponActor : public AZCItemActor
{
	GENERATED_BODY()

public:
	AZCWeaponActor();
	
	/*===========오버라이드 함수들===========*/
protected:
	virtual void PostInitializeComponents() override;
	//IZCItem인터페이스 오버라이드
	virtual const FZCItemTable* GetInfo() const override { return WeaponInfo; }

public:
	virtual void Initialize(FZCItemTable* NewItem) override;
	
	/*===========Getter===========*/
public:
	FORCEINLINE const struct FZCWeaponTable* GetWeaponInfo() const { return WeaponInfo; }
	FORCEINLINE const EWeaponType GetWeaponType() const { return WeaponInfo->WeaponType; }

	/*===========공격 판정 트레이스===========*/
public:
	/**
	 * 공격 판정이 가능한지 여부
	 * @param bCanHit - 공격 판정 가능 여부
	 * @param bAllowMultipleHit - 다단 히트 여부
	 * @param HitInterval - 다단 히트 간격
	 */
	void CanHit(bool bCanHit, bool bAllowMultipleHit, float HitInterval);

	void SetAttackInfo(EAttackType NewAttackType, uint8 NewGauge) { AttackType = NewAttackType; AttackChargingGauge = NewGauge; }

protected:
	void OnHitActor(AActor* HitActor, const FHitResult& HitResult);

	/*===========무기 구성===========*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UZCHitTraceComponent* HitTraceComponent;


	/*===========무기 정보===========*/
protected:
	struct FZCWeaponTable* WeaponInfo;

private:
	EAttackType AttackType = EAttackType::Normal;

	uint8 AttackChargingGauge = 0;
};
