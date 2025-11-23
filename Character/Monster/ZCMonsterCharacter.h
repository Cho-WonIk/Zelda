// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ZCCharacter.h"
#include "Interface/ZCAIInterface.h"
#include "GameData/Enum/ZCMonster.h"
#include "ZCMonsterCharacter.generated.h"

class UWidgetComponent;

class UZCMonsterStateComponent;
class UZCLockTargetComponent;
class AZCMonsterGroupActor;

UENUM(BlueprintType)
enum class EIdleType : uint8
{
	None			UMETA(DisplayName = "없음"),
	Standing		UMETA(DisplayName = "서있기"),
	Patrol			UMETA(DisplayName = "순찰"),
	LookAround		UMETA(DisplayName = "정찰"),
	Talk			UMETA(DisplayName = "대화"),
};

UCLASS()
class ZELDA_API AZCMonsterCharacter : public AZCCharacter, public IZCAIInterface
{
	GENERATED_BODY()
	
public:
	AZCMonsterCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/*===== IZCAIInterface =====*/
public:
	virtual void AttackByAI_Implementation(const FVector& TargetLocation = FVector::ZeroVector) override;
	virtual void DefenseByAI_Implementation(const FVector& TargetLocation = FVector::ZeroVector) override;

	virtual void EquipWeaponByAI(AZCWeaponActor* Weapon) override;
	virtual void EquipShieldByAI(AZCShieldActor* Shield) override;

	virtual EMonsterRole GetMonsterRole() const override { return MonsterRole; }
	virtual EMonsterType GetMonsterType() const override { return MonsterType; }

	/*===== Getter 함수 =====*/
public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE UZCMonsterStateComponent* GetMonsterStateComponent() const { return ZCMonsterStateComponent; } // 몬스터 상태 컴포넌트 반환

	/*===== 스폰 로직 =====*/
public:
	void SetIdleType(EIdleType NewIdleType) { IdleType = NewIdleType; } // Idle 타입 설정
	//void SetGroup(AZCMonsterGroupActor* NewGroup) { MonsterGroup = NewGroup; } // 몬스터 그룹 설정
	void SetPlacMonsterOnTalk(uint8 NewPlaceMonsterOnTalk) { PlaceMonsterOnTalk = NewPlaceMonsterOnTalk; } // Talk 시 몬스터 Idle 애니메이션 ID값 설정

	/*=================캐릭터 공통 동작===============*/
protected:
	// 캐릭터가 죽었을 때 호출되는 함수
	virtual void OnDeath() override;

	/*===== Monster Info =====*/
protected:
	// 몬스터 타입
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
	EMonsterRole MonsterRole = EMonsterRole::None;

	// 몬스터 종류
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
	EMonsterType MonsterType = EMonsterType::None;
	
	// 몬스터 Idle 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Idle")
	EIdleType IdleType = EIdleType::None;

	// Talk 시 몬스터 Idle  애니메이션 ID값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Idle", meta = (DisplayName = "Talk 시 몬스터 배치", EditCondition = "IdleType == EIdleType::Talk"))
	uint8 PlaceMonsterOnTalk = 255;

	// 몬스터가 처음 스폰된 위치
	UPROPERTY(VisibleAnywhere, Category = "Monster|Idle", meta = (AllowPrivateAccess = "true", HideInDetailPanel))
	FVector SpawnLocation = FVector::ZeroVector;

	// 캐릭터 구성
protected:
	class UZCMonsterStateComponent* ZCMonsterStateComponent;

	// 몬스터 락온 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<UZCLockTargetComponent> LockOnComponent;

	// UI 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> LifeBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> AggroIndicator;


protected:
	// Talk 시 Idle 애니메이션 몽타주 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage)
	TArray<class UAnimMontage*> IdleTalkMontages;

	// Attack 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage)
	TObjectPtr<class UAnimMontage> AttackMontage;

	// Defense 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage)
	TObjectPtr<class UAnimMontage> DefenseMontage;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Group", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AZCMonsterGroupActor> MonsterGroup; // 몬스터 그룹

private:
	// 죽음 후 딜레이 타이머 핸들
	FTimerHandle OnDeathTimerHandle;
};
