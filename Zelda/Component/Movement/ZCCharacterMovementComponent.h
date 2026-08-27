// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Physics/ZCCollision.h"
#include "ZCCharacterMovementComponent.generated.h"

/*
bool CheckFloor(FHitResult& FloorHit) const 함수는 ECC_Visibility를 통해 보다 시각적으로 바닥을 체크할 수 있도록 하였다.

#include "ZCClimbTrace.h"에 선언된 트레이스 채널을 통해 클라이밍 가능한 벽을 탐색

커스텀 표면등반 캐릭터 무브먼트 컴포넌트

캡슐 컴포넌트를 활용하여 ECC_WorldStatic 채널을 멀티 트레이싱한 결과물과 캐릭터의 눈 높이에서 진행한 구체 트레이싱 결과물을 좋합하여

클라이이밍 가능한 벽을 탐색하고, 벽에 붙어 표면등반을 진행하는 컴포넌트

지속적으로 표면의 법선과 위치를 업데이트해 가능한 캐릭터가 벽에 자연스럽게 붙어있도록 한다.
벽에 붙어있는 상태에서 점프를 누르면 대쉬를 시도한다.

bool IsClimbing()을 통해 표면등반 중인지를 확인할 수 있다.
void TryClimbing() 을 통해 표면등반을 시도한다.
void CancelClimbing() 을 통해 표면등반을 강제로 끝낼 수 있다.
bool CanStartClimbing() 을 통해 표면등반을 시작할 수 있는지 확인할 수 있다.

글라이딩 캐릭터 무브먼트 컴포넌트

글라이딩 상태에서 캐릭터의 중력 비율을 조정하고, 공중에서의 이동 속도를 조정하는 로직

bool IsGliding()를 통해 글라이딩 중인지 확인 할 수 있다
bool CanStartGliding()을 통해 글라이딩을 시도할 수 있다.
void TryGliding()을 통해 글라이딩을 시도한다.
void CancelGliding()을 통해 글라이딩을 취소한다.
*/



constexpr ECollisionChannel ClimbTrace = Zelda::Channel::Climbable;

UENUM(BlueprintType)
enum ECustomMovementMode : uint8
{
	CMOVE_Climbing      UMETA(DisplayName = "Climbing"),
	CMOVE_Gliding       UMETA(DisplayName = "Gliding"),
	CMOVE_RidingGear    UMETA(DisplayName = "RidingGear"),
	CMOVE_MAX			UMETA(Hidden),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMovementDelegate, EMovementMode, PreviousMovementMode, ECustomMovementMode, PreviousCustomMode);

DECLARE_DELEGATE_OneParam(FOnCanClimbDownCliffDelegate, bool);

UCLASS()
class ZELDA_API UZCCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UZCCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

public:
	FOnMovementDelegate OnMovementChangeDelegate;				// 커스텀 이동 모드 변경시 델리게이트
	FOnCanClimbDownCliffDelegate OnCanClimbDownCliffDelegate;		// MOVE_Walking 혹은 MOVE_NavWalking 상태일때 클라이밍 다운 동작이 가능함을 알림

	// Override functions
protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual float GetMaxSpeed() const override;

	virtual float GetMaxAcceleration() const override;

	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	virtual void PhysWalking(float deltaTime, int32 Iterations) override;

private:
	bool CheckFloor(FHitResult& FloorHit) const;		// 바닥 체크

	float Restore_GravityScale = 0.0f;					// 기존 중력 비율
	bool Restore_bOrientRotationToMovement = false;		// 기존 캐릭터 Yaw회전 설정
	FRotator Restore_RotationRate;						// 기존 캐릭터 회전 속도

	/* ======================================= Walk상태 로직들 ========================================= */
public:
	// 달리기 로직
	UFUNCTION(BlueprintPure)
	bool IsRunning() const { return bIsRunning && IsMovingOnGround() && UpdatedComponent; }		// 달리기 중인지 확인

	UFUNCTION(BlueprintCallable)
	void SetWantsToRun(bool bWantsToRun) { bWantsToRun ? StartRunning() : StopRunning(); }		// 달리기 하고 싶은지 여부 설정

	UFUNCTION(BlueprintCallable)
	void StartRunning();							// 달리기 시작

	UFUNCTION(BlueprintCallable)
	void StopRunning();								// 달리기 중지

public:
	// 주목상태
	UFUNCTION(BlueprintCallable)
	void SetAttentionState(bool bIsAttention);		// 주목 상태 설정

public:
	UPROPERTY(Category = "Character Movement: Attention", EditAnywhere, meta = (DisplayName = "주목 상태 이동 속도"))
	float AttentionSpeed = 230.0f;					// 주목 상태 이동 속도

private:
	bool bIsRunning = false;						// 달리기 중인지 여부

	float Restore_MaxWalkSpeed = 0.0f;				// 기존 달리기 속도

	UPROPERTY(Category = "Character Movement: Running", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "10000.0", DisplayName = "달리기 속도"))
	float RunningSpeed = 800.0f;					// 달리기 속도

	/* ======================================= 기어 탑승 로직들 ========================================= */
public:
	UFUNCTION(BlueprintPure)
	bool IsRidingGear() const { return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_RidingGear && UpdatedComponent; }

	UFUNCTION(BlueprintCallable)
	bool CanStartRidingGear();								// 기어 탑승 가능 상태인지 확인

	UFUNCTION(BlueprintCallable)
	void TryRidingGear(AActor* TargetGear, FName Socket);	// 기어 탑승 시도

	UFUNCTION(BlueprintCallable)
	void CancelRidingGear();								// 기어 탑승 취소

	void SetCanRideGear(bool bCanRide) { bCanRideGear = bCanRide; } // 캐릭터가 입력 기어 앞에 있는지 설정

private:
	UFUNCTION(BlueprintCallable)
	void PhysRidingGear(float DeltaTime, int32 Iterations);	// 기어 탑승 피직스

	bool ShouldStopRidingGear() const;

	void StopRidingGear(float DeltaTime, int32 Iterations);

private:
	TWeakObjectPtr<AActor> CurrentRidingGear;

	// 캐릭터가 부착될 액터의 소켓 이름
	FName GearAttachSocketName = NAME_None;

	// 플레이어가 기어에 탑승을 원하는지 여부
	bool bWantsToRidingGear = false;

	// 캐릭터에서 입력 기어 앞에 있는지 받아옴
	bool bCanRideGear = false;


	/* ======================================= 글라이딩 로직들 ========================================= */
public:
	UFUNCTION(BlueprintPure)
	bool IsGliding() const { return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_Gliding && UpdatedComponent; }

	UFUNCTION(BlueprintCallable)
	bool CanStartGliding();										// 글라이딩 가능 상태인지 확인

	UFUNCTION(BlueprintCallable)
	void TryGliding();											// 글라이딩 시도

	UFUNCTION(BlueprintCallable)
	void CancelGliding();										// 글라이딩 취소

private:
	UFUNCTION(BlueprintCallable)
	void PhysGliding(float DeltaTime, int32 Iterations);		// 글라이딩 피직스

	bool ShouldStopGliding() const;

	bool GlidingDownToFloor() const;

	void StopGliding(float DeltaTime, int32 Iterations);

	FVector ComputeGlidingVelocity(float DeltaTime, const FVector& FallAcceleration);

private:
	UPROPERTY(Category = "Character Movement: Gliding", EditAnywhere, meta = (DisplayName = "글라이딩 중력 배율"))
	float GlidingGravityScale = 0.3f;							// 글라이딩 시 중력 비율

	UPROPERTY(Category = "Character Movement: Gliding", EditAnywhere, meta = (DisplayName = "글라이딩 최대 낙하 속도"))
	float MaxGlidingFallingSpeed = 140.0f;						// 글라이딩 시 최대 낙하 속도

	UPROPERTY(Category = "Character Movement: Gliding", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "10000.0", DisplayName = "글라이딩 수평 속도"))
	float MaxGlidingSpeed = 500.0f;								// 글라이딩 속도

	UPROPERTY(Category = "Character Movement: Gliding", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "10000.0", DisplayName = "글라이딩 가속도"))
	float MaxGlidingAcceleration = 1100.0f;						// 글라이딩 가속도

	UPROPERTY(Category = "Character Movement: Gliding", EditAnywhere, meta = (DisplayName = "글라이딩 공중 컨트롤 적용 여부"))
	bool bGlidingHasLimitedAirControl = true;					// 글라이딩 중 공중 제어 제한 여부	

	UPROPERTY(Category = "Character Movement: Gliding", EditAnywhere, meta = (DisplayName = "글라이딩 공중 컨트롤 속도"))
	float GlidingControlSpeed = 3000.0f;						// 글라이딩 공중 제어 속도

	UPROPERTY(Category = "Character Movement: Gliding", EditAnywhere, meta = (DisplayName = "글라이딩 공중 회전 속도"))
	FRotator GlidingRotationRate = FRotator(0.0f, 10.0f, 0.0f);


	float VERTICAL_SLOPE_NORMAL_Z = 0.001f;						// 수직 경사면의 법선 Z값, UCharacterMovementComponent 값

	bool bWantsToGliding = false;

	/* ======================================= 표면등반 로직들 ========================================= */
public:
	UFUNCTION(BlueprintPure)
	bool IsClimbing() const { return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_Climbing && UpdatedComponent; }

	UFUNCTION(BlueprintPure)
	bool IsClimbDashing() const { return IsClimbing() && bIsClimbDashing; }

	UFUNCTION(BlueprintPure)
	FVector GetClimbSurfaceNormal() const { return CurrentClimbingNormal; }

	UFUNCTION(BlueprintPure)
	FVector GetClimbDashDirection() const { return ClimbDashDirection; }

	UFUNCTION(BlueprintCallable)			// 표면등반 되는지 확인 후 표면등반
	void TryClimbing();

	UFUNCTION(BlueprintCallable)			// 표면등반 대쉬가 되는지 확인후 표면등반 대쉬
	void TryClimbDashing();

	UFUNCTION(BlueprintCallable)			// 표면등반 로직 취소
	void CancelClimbing();

	UFUNCTION(BlueprintCallable)
	bool CanStartClimbing();				// 표면등반 가능 상태인지 확인

	UFUNCTION(BlueprintCallable)
	void TryClimbDownCliff();				// 클라이밍 다운 시도

	FVector GetClimbClimbDownSurface() const { return ClimbClimbDownSurface; }	// 클라이밍 다운 시 표면 위치 정보 반환


private:
	void SweepAndStoreWallHits();									// 캡슐모양의 트레이스를 통해 벽을 탐색하고, 벽의 정보를 제공해주는 함수

	bool CheckCliffEdge();											// 낭떠러지 확인하는 함수

	void ComputeSurfaceInfo();										// 평균 법선과 노멀을 계산하는 함수

	bool EyeHeightTrace(const float TraceDistance) const;

	bool IsFacingSurface(float Steepness) const;

	void SnapToClimbingSurface(float DeltaTime) const;				// 캐릭터가 벽의 법선 벡터에 맞게 정렬할 수 있도록 해주는 함수

	void PhysClimbing(float DeltaTime, int32 Iterations);			// 표면등반 피직스

	void ComputeClimbingVelocity(float DeltaTime);					// 클라이밍 대시 속도 계산

	void MoveAlongClimbingSurface(float DeltaTime);					// 표면 정보를 바탕으로 이동

	bool ClimbDownToFloor() const;

	void SetRotationToStand() const;

	bool TryClimbUpLedge() const;									// ledge인지 검사 후 ledge이면 클라이밍 종료

	bool HasReachedEdge() const;

	bool CanMoveToLedgeClimbLocation() const;

	bool IsLocationWalkable(const FVector& CheckLocation) const;	// 클라이밍으로 바닥에 도달했는지 확인, ECC_WorldStatic채널을 사용

	bool ShouldStopClimbing() const;

	void StopClimbing(float DeltaTime, int32 Iterations);

	FQuat GetClimbingRotation(float DeltaTime) const;

	void UpdateClimbDashState(float DeltaTime);						// 클라이밍 대시 상태 업데이트 함수(Curve에 따라 속도 조정)

	void AlignClimbDashDirection();

	void StoreClimbDashDirection();

	void StopClimbDashing();

private:
	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (DisplayName = "캡슐 반지름"))
	int32 CollisionCapsuleRadius = 50;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (DisplayName = "캡슐 절반 높이"))
	int32 CollisionCapsuleHalfHeight = 72;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "500.0", DisplayName = "최대 등반 속도"))
	float MaxClimbingSpeed = 120;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "2000.0", DisplayName = "최대 등반 가속도"))
	float MaxClimbingAcceleration = 380;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "3000.0", DisplayName = "등반 제동 감속도"))
	float BrakingDecelerationClimbing = 550.0f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "60.0", DisplayName = "표면 스냅 속도"))
	float ClimbingSnapSpeed = 4.0f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "80.0", DisplayName = "표면 거리 유지값"))
	float DistanceFromSurface = 45.0f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "1.0", ClampMax = "60.0", DisplayName = "등반 회전 속도"))
	int32 ClimbingRotationSpeed = 5;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "80.0", DisplayName = "충돌 캡슐 축소값"))
	float ClimbingCollisionShrinkAmount = 30;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "1.0", ClampMax = "500.0", DisplayName = "바닥 검사 거리"))
	float FloorCheckDistance = 90.0f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "1.0", ClampMax = "75.0", DisplayName = "등반 가능한 최소 수평 각도"))
	float MinHorizontalDegreesToStartClimbing = 25.0f;

	UPROPERTY(Category = "Character Movement: Climbing", EditDefaultsOnly, meta = (DisplayName = "난간 오르기 몽타주"))
	TObjectPtr<class UAnimMontage> LedgeClimbMontage;

	UPROPERTY(Category = "Character Movement: Climbing", EditDefaultsOnly, meta = (DisplayName = "낭떨어지 클라이밍 몽타주"))
	TObjectPtr<class UAnimMontage> CliffClimbMontage;

	UPROPERTY(Category = "Character Movement: Climbing", EditDefaultsOnly, meta = (DisplayName = "등반 대쉬 커브"))
	TObjectPtr<class UCurveFloat> ClimbDashCurve;

	UAnimInstance* AnimInstance;

	TArray<FHitResult> CurrentWallHits;

	FCollisionQueryParams ClimbQueryParams;

	bool bWantsToClimb = false;

	bool bIsClimbDashing = false;

	float CurrentClimbDashTime;

	FVector ClimbDashDirection;

	FVector CurrentClimbingNormal;

	FVector CurrentClimbingPosition;

	FVector ClimbClimbDownSurface;

	// 클라이밍 쿨타임
	float LastClimbingEndTime = -1.0f;
	float ClimbingCooldownDuration = 1.0f;
};
