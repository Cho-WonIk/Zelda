// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "Physics/ZCCollision.h"
#include "Struct/Enum/ZCDirection.h"
#include "Struct/Enum/ZCItemType.h"
#include "Interface/ZCMontageInterface.h"
#include "Interface/ZCCombatInterface.h"
#include "ZCCharacter.generated.h"

namespace BoneSocket
{
	namespace Weapon
	{
		static const FName Hand			= TEXT("Socket_hand_r");
		static const FName Holster		= TEXT("Socket_Holster_Weapon");
	}

	namespace Shield
	{
		static const FName Hand			= TEXT("Socket_hand_l");
		static const FName Holster		= TEXT("Socket_Holster_Shield");
	}

	namespace IK
	{
		static const FName RightHand	= TEXT("hand_r");
		static const FName LeftHand		= TEXT("hand_l");
		static const FName RightFoot	= TEXT("foot_r");
		static const FName LeftFoot		= TEXT("foot_l");
	}
}

namespace MotionWarping
{
	namespace WarpTarget
	{
		static const FName AttackTarget = TEXT("AttackTarget");
		static const FName ClimbDownTarget = TEXT("ClimbDown");
	}
}


class AZCWeaponActor;
class AZCShieldActor;
class UZCStateComponent;
class UZCHitReactionComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponTypeChangedDelegate, EWeaponType);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShieldTypeChangedDelegate, EShieldType);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGuardStateChangedDelegate, bool);

UCLASS()
class ZELDA_API AZCCharacter : public ACharacter, public IGenericTeamAgentInterface, public IZCMontageInterface, public IZCCombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZCCharacter(const FObjectInitializer& ObjectInitializer);

	/*=================델리 게이트===============*/
public:
	// 무기 타입 변경
	FOnWeaponTypeChangedDelegate OnWeaponTypeChanged;
	// 방패 타입 변경
	FOnShieldTypeChangedDelegate OnShieldTypeChanged;
	// 가드 상태 변경
	FOnGuardStateChangedDelegate OnGuardStateChanged;

public:
	static FName StateComponentName;

	/*=================인터페이스===============*/
public:
	// IGenericTeamAgentInterface 인터페이스
	// 255는 중립(NPC), 0은 플레이어, 1은 플레이어 아군 NPC, 나머지는 몬스터로 지정
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override { TeamID = NewTeamID; }

	// IZCMontageInterface 인터페이스
	virtual void CanGotoNextSection(const FName MontageName, const FName NextSectionName) override;
	virtual void DrawEquipmentNotify(EItemType Type) override;
	virtual void SheathEquipmentNotify(EItemType Type) override;

	// IZCCombatInterface 인터페이스
	virtual void OnTraceHit(bool bCanHit, bool bAllowMultipleHit, float HitInterval) override;
	virtual void OnParry(bool bCanParry) override;

	/*=================Getter 함수===============*/
public:
	AZCWeaponActor* GetCurrentWeapon() const { return CurrentWeapon; }
	AZCShieldActor* GetCurrentShield() const { return CurrentShield; }

	UFUNCTION(BlueprintPure)
	UZCStateComponent* GetStateComponent() const { return StateComponent; }

	UFUNCTION(BlueprintPure)
	UZCHitReactionComponent* GetHitReactionComponent() const { return HitReactionComponent; }

	/*=================오버라이드 함수들===============*/
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual float InternalTakeRadialDamage(float Damage, struct FRadialDamageEvent const& RadialDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/*=================캐릭터 공통 동작===============*/
public:
	bool IsCritialBone(const FName& BoneName) const { return CriticalBones.Contains(BoneName); }

protected:
	// 캐릭터가 죽었을 때 호출되는 함수
	virtual void OnDeath();

	// 캐릭터가 그로시 상태일 때 호출되는 함수
	virtual void OnStagger();

protected:
	// 애니메이션 몽타주 재생
	void GotoNextMontageSection(const FName NextMontageName);


	/*==================아이템 장착 / 분리===================*/
protected:
	// 아이템 부착
	void AttachActorToSocket(const FName SocketName, AActor* Item);
	// 아이템 분리 함수, 캐릭터에 참조된 포인터 변수 nullptr로 초기화
	void DetachActorFromSocket(AActor* Item);

	/*==================무기 장착===================*/
public:
	void SetNewWeapon(AZCWeaponActor* NewWeapon);
	void SetNewShield(AZCShieldActor* NewShield);

protected:
	// 무기 장착
	void EquipWeapon();
	void UnEquipWeapon();

	// 방패 장착
	void EquipShield();
	void UnEquipShield();

protected:
	// 캐릭터가 죽었는지 여부
	UPROPERTY(VisibleAnywhere, Category = "Monster", meta = (AllowPrivateAccess = "true", HideInDetailPanel))
	bool bIsDead = false;

protected:
	bool bIsEquipWeapon = false; // 무기 손에 장착 여부
	bool bIsEquipShield = false; // 방패 손에 장착 여부

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	class AZCWeaponActor* CurrentWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield")
	class AZCShieldActor* CurrentShield = nullptr;

protected:
	// 팀 ID 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team", meta = (AllowPrivateAccess = "true"))
	FGenericTeamId TeamID;

protected:
	// 크리티컬 판정이 발생하는 본
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bone", meta = (DisplayName = "크리티컬 본"))
	TSet<FName> CriticalBones;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TObjectPtr<UZCStateComponent> StateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping")
	TObjectPtr<class UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Control")
	TObjectPtr<class UPhysicsControlComponent> PhysicsControlComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	TObjectPtr<UZCHitReactionComponent> HitReactionComponent;

};
