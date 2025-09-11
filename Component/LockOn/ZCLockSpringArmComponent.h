// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"

#include "Kismet/GameplayStatics.h"

#include "ZCLockSpringArmComponent.generated.h"

/*
 * 락온 스프링암 컴포넌트
 * - 락온 타겟을 바라보도록 회전하는 스프링암 컴포넌트
 * - 락온 타겟을 바라보도록 카메라를 회전시키는 기능을 제공
 * - 락온 타겟을 전환하는 기능을 제공
 * - 락온 타겟의 위치에 락온 위젯을 표시하는 기능을 제공
 */

UENUM(BlueprintType)
enum class ETargetSwitchDirection : uint8
{
	Right	UMETA(DisplayName = "SwitchRight"),
	Up		UMETA(DisplayName = "SwitchUp"),
	Left	UMETA(DisplayName = "SwitchLeft"),
	Down	UMETA(DisplayName = "SwitchDown"),
};

// 캐릭터의 회전모드(타겟을 따라 회전할지, 이동방향을 따라 회전할지)
UENUM(BlueprintType)
enum class ECharacterOrientationMode : uint8
{
	None				UMETA(DisplayName = "None"),
	OrientToMovement	UMETA(DisplayName = "OrientToMovement"),	// 이동방향을 따라 캐릭터 회전
	OrientToTarget		UMETA(DisplayName = "OrientToTarget"),		// 타겟을 바라보도록 캐릭터 회전
};

class UZCLockTargetComponent;

UCLASS(meta = (BlueprintSpawnableComponent))
class ZELDA_API UZCLockSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	UZCLockSpringArmComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FRotator FindDesiredRotation(float DeltaTime);
	FVector MimickCameraPositionFromControlRotation(FRotator Rotation, float DeltaTime);

	bool UpdateLookExt(FVector2D LookAxisVector, float TargetSwitchThreshold);

	void BreakLockAndFindAnother();

	UZCLockTargetComponent* FindBestTarget();

	APlayerCameraManager* GetPlayerCameraManager(int32 PlayerIdx = 0) const { return UGameplayStatics::GetPlayerCameraManager(GetWorld(), PlayerIdx); }

public:
	FIntPoint GetDefaultWidgetDrawSize() const { return DefaultWidgetDrawSize; }

	bool GetDefaultWidgetDrawAtDesiredSize() const { return bDefaultWidgetDrawAtDesiredSize; }

	TSubclassOf<UUserWidget> GetDefaultWidgetClass() { return LockIndicatorWidgetClass; }

public:
	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	bool UpdateLookMouse(FVector2D LookAxisVector) { return UpdateLookExt(LookAxisVector, TargetSwitchMouseDeltaThreshold); }

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	bool UpdateLookGamepad(FVector2D LookAxisVector) { return UpdateLookExt(LookAxisVector, TargetSwitchGamepadDeltaThreshold); }

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	void ToggleLock();

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	void SwitchTarget(ETargetSwitchDirection SwitchDirection);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "LockTarget_Function")
	bool IsLocked() { return CurrentTarget != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	void LockTarget(UZCLockTargetComponent* TargetComponent);

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	void BreakLock();

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	TArray<UZCLockTargetComponent*> GetTargetComponents(FVector SourceLocation, float MaxDistance, bool Unfiltered = false);

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	UZCLockTargetComponent* GetCurrentTarget() const { return CurrentTarget; }

	FTransform GetTargetTransform() const;

protected:
	// Lock상태일때 카메라 및 플레이어의 회전값을 LockTarget의 회전값으로 맞출지 여부
	// true일 경우 카메라가 타켓 방향으로 회전을 안함, 캐릭터가 회전을 안함
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "카메라 및 캐릭터 회전 제한 여부"))
	bool bDisableRotation;

	// 락온을 할 수 있는 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "락온 시 최대 거리"))
	float MaxTargetLockDistance;

	// 락온을 할 수 있는 최소 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "락온 해제 거리"))
	float MaxTargetLockBreakDistance;

	// 락온이 외부요인(사망, 거리 초과 등등)에 의해 해제되었을때 자동으로 다른 타겟 락온 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "자동으로 새 타겟에 락온할지"))
	bool bAutoLockNewTargets;

	// 마우스 입력 변화량이 초과하면 다른 타겟으로 전환
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "마우스 감도 기준"))
	float TargetSwitchMouseDeltaThreshold;

	// 게임패드 입력 변화량이 초과하면 다른 타겟으로 전환
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "게임패드 감도 기준"))
	float TargetSwitchGamepadDeltaThreshold;

	// 타겟으로 전환하는 최소 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "전환 딜레이 시간"))
	float TargetSwitchMinDelaySeconds;

	// 락온 중 플레이어 캐릭터 회전 제어 방식
	// None일 경우 플레이어의 회전을 제어 안함
	// OrientToMovement일 경우 플레이어의 회전을 이동방향으로 맞춤
	// OrientToTarget일 경우 플레이어의 회전을 타겟 방향으로 맞춤
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "캐릭터 회전 방식"))
	ECharacterOrientationMode CharacterOrientationMode;

	// 카메라의 회전 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "락온시 회전 속도"))
	float LockOnRotationRate;

	// 락온 위젯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "기본 위젯 클래스"))
	TSubclassOf<UUserWidget> LockIndicatorWidgetClass;

	// 락온 위젯의 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "위젯 기본 사이즈"))
	FIntPoint DefaultWidgetDrawSize;

	// 락온 위젯의 크기 변경 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "락온 설정", meta = (AllowPrivateAccess = "true", DisplayName = "자동 사이즈 조절 여부"))
	bool bDefaultWidgetDrawAtDesiredSize;

	// 현재 락온된 타겟
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "락온 상태", meta = (AllowPrivateAccess = "true", DisplayName = "현재 락온 중인 대상"))
	UZCLockTargetComponent* CurrentTarget;

	// 소유 캐릭터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "락온 상태", meta = (AllowPrivateAccess = "true", DisplayName = "락온된 캐릭터"))
	ACharacter* OwningPlayer;

private:
	// 마지막으로 타겟 전환이 발생한 시간
	float LastTargetSwitchTime;

	// 락 온 타겟이 없으면 다시 카메라를 캐릭터의 전방으로 초기화할지 여부
	bool ShouldAlignCameraToForward;

	// 카메라회전 속도값
	FRotator ForwardAlignRot;

	// 캐릭터 정보를 저장
	bool Restore_bOrientRotationToMovement;
	bool Restore_bUseControllerDesiredRotation;

};
