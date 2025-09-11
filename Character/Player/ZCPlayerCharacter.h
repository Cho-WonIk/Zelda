// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ZCCharacter.h"
#include "Struct/Enum/ZCItemType.h"
#include "ZCPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
struct FInputActionValue;

class UZCCharacterMovementComponent;
class UZCLockSpringArmComponent;
class UZCPlayerStateComponent;

enum ECustomMovementMode : uint8;

namespace BoneSocket
{
	namespace Glide
	{
		// 캐릭터 글라이더 소켓
		const FName GlideGrip	= TEXT("Socket_Glide");
		const FName Holster		= TEXT("Socket_Holster_Glide");

		// 글라이더 매쉬 소켓 IK용
		const FName LeftHandIK	= TEXT("LeftHand");
		const FName RightHandIK = TEXT("RightHand");
	}
}

UCLASS()
class ZELDA_API AZCPlayerCharacter : public AZCCharacter
{
	GENERATED_BODY()
	
public:
	AZCPlayerCharacter(const FObjectInitializer& ObjectInitializer);

/*==================오버라이드===================*/
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void Tick(float DeltaTime);
	virtual void PostInitializeComponents() override;

/*=================인터페이스===============*/
// IZCMontageInterface 인터페이스
public:
	virtual void CanGotoNextSection(const FName MontageName, const FName NextSectionName) override;

/*==================델리게이트 바인딩===================*/
protected:
	// 캐릭터 이동 모드 변경시 호출
	UFUNCTION()
	void OnMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode);

	// 캐릭터가 낭떨어지에서 내려갈 수 있는지 여부
	UFUNCTION()
	void OnCanClimbDownCliff(const bool bDownCliff);

/*==================Getter 함수들===================*/
public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE UZCCharacterMovementComponent* GetZCCharacterMovementComponent() const { return ZCCharacterMovementComponent; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE UZCPlayerStateComponent* GetZCPlayerStateComponent() const { return ZCPlayerStateComponent; }

	// 글라이더 IK용, 애님인스턴스 블루프린트용
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "IK|Glide")
	FTransform GetGlideLeftHandIK() const;
	// 글라이더 IK용, 애님인스턴스 블루프린트용
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "IK|Glide")
	FTransform GetGlideRightHandIK() const;

/*==================캐릭터 컴포넌트===================*/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UZCLockSpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	// 글라이더
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Glide, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> GlideMeshComponent;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UZCCharacterMovementComponent* ZCCharacterMovementComponent;

	// 플레이어 상태 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UZCPlayerStateComponent* ZCPlayerStateComponent;

/*==================애니 몽타주===================*/
protected:
	// 웅크리기 대시
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TMap<EZCDirection, class UAnimMontage*> CrouchEvasion;

	// 선자세 회피
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TMap<EZCDirection, class UAnimMontage*> StandEvasion;

	// 공격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TMap<EWeaponType, class UAnimMontage*> AttackMontages;

	// 공격 차징 중
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TMap<EWeaponType, class UAnimMontage*> AttackChargingMontages;

	// 차징 공격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TMap<EWeaponType, class UAnimMontage*> ChargeAttackMontages;

	// 패리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> ParryMontage;

	// 무기 장착
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> DrawWeaponMontage;

	// 무기 해제
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> SheathWeaponMontage;


/*==================입력 액션, XBOX 컨트롤러 기준===================*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;			// R스틱

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;			// L스틱

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;			// Y버튼

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackAction;			// X버튼

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteraactionAction;	// B버튼

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RunAction;				// A버튼

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttentionAction;		// LT버튼

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CrouchAction;			// LS버튼

/*==================입력 바인딩===================*/
protected:
	// L스틱 입력
	void Move(const FInputActionValue& Value);
	// R스틱 입력
	void Look(const FInputActionValue& Value);

	// Y버튼 입력
	virtual void Jump() override;
	virtual void StopJumping() override;

	// X버튼 입력
	void StartAttack();
	void NormalAttack();
	void AttackCharging();
	void HoldAttack();
	void FinishAttack();	// 입력바인딩은 안되었지만 공격 종료 시 호출되는 함수

	// B버튼 입력
	void Interaction();

	// A버튼 입력
	void StartRunning();
	void Running();
	void StopRunning();

	// LT버튼 입력
	void StartAttention();
	void StopAttention();

	// LS버튼 입력
	void ToggleCrouch();

/*==================입력 버퍼===================*/
protected:
	// 입력 버퍼에 입력 등록
	void BufferInput(const FName InputName) { InputBufferMap.Add(InputName, GetWorld()->GetTimeSeconds()); }
	// 입력 버퍼에 등록된 입력을 소비
	bool ConsumeBufferInput(const FName InputName);
	void ClearBufferInput(const FName InputName) { InputBufferMap.Remove(InputName); } // 특정 입력 버퍼 초기화
	void ClearAllBufferInput() { InputBufferMap.Empty(); } // 모든 입력 버퍼 초기화
private:
	UPROPERTY(EditAnywhere, Category = "Input Buffer")
	float InputBufferDuration = 1.0f;

	TMap<FName, float> InputBufferMap; // 입력 버퍼 맵

/*==================달리기===================*/
protected:
	bool bIsRunning = false; // 달리기 중인지 여부

/*==================주목===================*/
private:
	bool bIsAttentionTriggered = false; // 주목 상태를 트리거했는지 여부(락온된 타겟이 없어도 true)
	bool Restore_bOrientRotationToMovement = false; // 캐릭터 회전 방식 복원 값
	bool Restore_bUseControllerDesiredRotation = false; // 캐릭터 회전 방식 복원 값

/*==================전투 로직 ===================*/
protected:
	// ZCCharacter에 구현된 EquipWeapon, UnEquipWeapon함수에 애니메이션 몽타주 재생 추가
	void DrawWeapon();
	void SheathWeapon();

private:
	FTimerHandle AttackTimerHandle; // 공격 초기화 타이머

	uint8 MaxAttackComboCount = 3; // 최대 연속 공격 횟수
	uint8 CurrentAttackComboCount = 0; // 현재 연속 공격 횟수
	uint8 AttackChargingGauge = 0; // 모으기 공격 게이지

	bool bIsPlayingEquipMontage = false; // 장착 애니메이션 재생 중인지 여부
	bool bIsPlayingSheathMontage = false; // 장착 해제 애니메이션 재생 중인지 여부

	bool bAttackInputLock = false; // 공격 입력 잠금
	bool bChargingAttack = false; // 모으기 중 (모으기 공격 전) 인지 여부

/*==================클라이밍 로직===================*/
protected:
	void Climbing();
	void ClimbingCancel();

private:
	bool bCanClimbDownCliff = false;	// 낭떠러지에서 내려갈 수 있는지 여부
	FTimerHandle ClimbTimerHandle;

/*=================글라이딩 로직==================*/
protected:
	void Gliding();
	void GlidingCancel();

/*==================유틸 함수들===================*/
protected:
	// 플레이어의 이동 방향
	EZCDirection GetInputDirection() const;
};
