// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Device/ZCDeviceSkillComponent.h"
#include "ZCUltrahandSkillComponent.generated.h"

// 울트라핸드 내부 상태 관리용
enum class EUltraHandState : uint8
{
	// 비활성화
	None = 0,

	// 액터 탐색
	Find,

	// 액터를 선택(단일)
	SelectedSingle,
	// 액터를 선택(복수)
	SelectedMulti,

	// 액터의 회전을 조정
	Manipulation,
};

class AZCActor;
class AZCMetaBallActor;
class AZCGizmoActor;
class UZCUltrahandWorldSubsystem;
class UPhysicsHandleComponent;

struct FInputActionValue;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZELDA_API UZCUltrahandSkillComponent : public UZCDeviceSkillComponent, public IZCUltrahandDeviceInterface
{
	GENERATED_BODY()
	
public:
	UZCUltrahandSkillComponent();

	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetupInputComponent(UEnhancedInputComponent* InputComponent) override;

	void SetPhysicsHandleComponent(UPhysicsHandleComponent* NewGraper);

public:
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	virtual void AddAssembleCandidate(AZCActor* OwnerActor, AZCActor* AddActor) override;
	virtual void RemoveAssembleCandidate(AZCActor* OwnerActor, AZCActor* RemoveActor) override;

	virtual void SetHUDSubsystem(UZCHUDLocalPlayerSubsystem* NewLocalSub) override;

protected:
	void UpdateState(EUltraHandState NewState);

	/* 입력 바인딩 */
	// 울트라 핸드 기본 액션
	virtual void Action() override;
	// 울트라 핸드 취소 액션
	virtual void Cancel() override;

	// 오브젝트 분리 액션
	void DetachObject();

	// 오브젝트 앞뒤 이동 액션
	void MoveForwardBackward(const FInputActionValue& Value);

	// 오브젝트 회전
	void StartManipulation(const FInputActionValue& Value);
	void StopManipulation(const FInputActionValue& Value);
	void Manipulation(const FInputActionValue& Value);
	void ManipulationResetRotation();

private:
	// Action버튼 내부 처리 함수 : Find 상태
	void Action_Find_Internal();
	// Action버튼 내부 처리 함수 : Select 상태
	void Action_Select_Internal();

	// 카메라 방향으로 트레이스 수행, 가장 가까운 액터 반환
	bool TraceObjectFromCamera(float Distance, FHitResult& OutHitResult, AZCActor** OutActor) const;

	/*=========Tick에서 호출==============*/
	// 오브젝트 위치를 카메라 기준으로 업데이트
	void UpdateGrabbedObjectPosition();

	/*=========Tick에서 호출==============*/
	// 플레이어 주변 상호작용 액터들 검색 후 오버레이 상태 업데이트
	void UpdateInteractableActors();

	/*=========Tick에서 호출==============*/
	// 플레이어가 잡은 액터를 기준으로 가장 가까운 오브젝트와의 상호작용 표시
	void UpdateSnabToNearestActor();

	void UpdateCrosshairTracking();

	/*=============Tick에서 호출================*/
	void UpdateManipulation();

	// 액터를 선택하고 오버레이 적용
	void SelectActor(AZCActor* Actor);
	// 액터 선택 해제
	void DeselectCurrentActor();

	// 액터 잡는 함수
	void GrabActorAtHitPoint(const FHitResult& HitResult);

	// 카메라 위치와 초기 오브젝트 위치 계산
	FVector CalculateTargetGrabLocation() const;

	// 오버레이 상태 업데이트
	void UpdateActorOverlayStates(const TSet<AZCActor*>& NewActors, const TSet<AZCActor*>& OldActors);

	// GluePoint 관련 로직들
	// 글루 포인트 스폰
	void SpawnGluePoint();

	// 글루 포인트 Visible 옵션
	void SetGluePointVisible(bool bVisible);

	// 글루 포인트 위치 조정 (두 지점 필요)
	void UpdateGluePointTransform(const FVector& SourceLocation, const FVector& TargetLocation);


	// 기즈모 관련 로직들
	// 기즈모 스폰
	void SpawnGizmo();

	// 기즈모 Visible옵션
	void SetGizmoVisible(bool bVisible);

	// 기즈모 위치 조정
	void UpdateGizmoTransform(const FVector& NewLocation);

	// 입력 액션
protected:
	// 접착 해제
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DetachObjectAction;

	// 오브젝트 앞뒤 이동
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveForwardBackwardAction;

	// 오브젝트 회전 트리거
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ManipulationModeAction;

	// 회전 : 홀드 중 오브젝트 회전
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ManipulationAction;

	// 회전 리셋 : 홀드 중 오브젝트 회전값을 CDO값으로 변경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ManipulationResetRotationAction;

protected:
	// 울트라 핸드로 잡을 수 있는 최대 거리
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Detection", meta = (ClampMin = "0.0", DisplayName = "최대 감지 거리"))
	float CatchMaxDistance = 1000.0f;

	// TraceObjectFromCamera 판정 스피어 반경
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Detection", meta = (ClampMin = "0.0", DisplayName = "감지 반경(Sphere)"))
	float CatchDetectionRadius = 20.0f;

	// 상호작용 가능한 오브젝트를 표시하는 오버레이 범위
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Interaction", meta = (ClampMin = "0.0", DisplayName = "상호작용 표시 범위"))
	float InteractionOverlayRange = 700.0f;

	// 물체를 당길 수 있는 최소 거리
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Movement", meta = (ClampMin = "0.0", DisplayName = "최소 잡기 거리"))
	float MinGrabDistance = 150.0f;

	// 앞뒤 이동 간격
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Movement", meta = (ClampMin = "0.0", DisplayName = "앞뒤 이동 간격"))
	float PushPullStep = 30.0f;

	// 회전 보간속도
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Movement", meta = (ClampMin = "0.1", DisplayName = "회전 보간 속도"))
	float RotationInterpSpeed = 10.0f;

	// 물체 조작 시 회전 각도
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Manipulation", meta = (DisplayName = "회전각도"))
	float AngleDeg = 45.0f;

	// 접착 가능 거리
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Glue", meta = (ClampMin = "0.0", DisplayName = "접착 가능 최소 거리"))
	float MinInteractionDistance = 200.0f;

	// 메쉬에 소켓이 존재할 경우 접착 유도가 되는 최대 거리
	UPROPERTY(EditAnywhere, Category = "Ultrahand|Glue", meta = (ClampMin = "0.0", DisplayName = "소켓 접착 유도 최대 거리"))
	float SnapSocketMaxDistance = 5.0f;

	// 기즈모 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "기즈모 액터 클래스"))
	TSubclassOf<class AZCGizmoActor> GizmoClass;

	// 접착 오브젝트 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "접착 오브젝트 클래스"))
	TSubclassOf<class AZCGlueActor> GlueActorClass;

	// 접착 포인트 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "접착 포인트 액터 클래스"))
	TSubclassOf<class AZCGluePointActor> GluePointActorClass;

	// 플레이어와 잡은 오브젝트 사이의 시각화
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultrahand|VFX", meta = (DisplayName = "잡기 연결 효과(VFX)"))
	TObjectPtr<class UNiagaraSystem> PlayerToGrabbedVFX;

protected:
	UPROPERTY()
	TObjectPtr<class AZCGluePointActor> GluePoint;

	UPROPERTY()
	TObjectPtr<class AZCGizmoActor> Gizmo;

	// 현재 상호작용 가능한 액터들
	UPROPERTY()
	TSet<AZCActor*> InteractableActors;

	// 울트라 핸드 결합 로직

	// 현재 잡은 액터
	AZCActor* SelectedActor = nullptr;

	TSet<AZCActor*> SelectedActorGroup;

	AZCActor* SnapSourceActor = nullptr;
	AZCActor* SnapTargetActor = nullptr;

	TWeakObjectPtr<UPrimitiveComponent> SnapSourceComponent = nullptr;
	TWeakObjectPtr<UPrimitiveComponent> SnapTargetComponent = nullptr;

	// 핸들 컴포넌트
	UPROPERTY()
	UPhysicsHandleComponent* GrabHandle = nullptr;

	// 스냅 로직을 위한 임시 저장 변수
	UPROPERTY()
	FVector SnapSourceLocation = FVector::ZeroVector; // 소스 액터의 접착 지점

	UPROPERTY()
	FVector SnapTargetLocation = FVector::ZeroVector; // 타겟 액터의 접착 지점

private:
	// 오브젝트를 잡았을대 카메라부터의 거리
	float InitialGrabDistance = 0.0f;

	// 카메라의 Yaw 회전 동기화를 위한 이전 프레임 Yaw값
	float LastCameraYaw = 0.0f;

	// 내부 상태
	EUltraHandState CurrentState;

	// 오브젝트를 잡은 상태에서 결합할 수 있는 액터 페어
	TArray<TPair<class AZCActor*, class AZCActor*>> AssembleCandidates;
	
	// 잡았을 때 유지할 물체의 '월드' 회전값 (카메라와 무관하게 고정)
	FQuat TargetWorldRotation;

	// 부드러운 회전을 위한 값
	FQuat SmoothedTargetWorldRotation;

	class UZCUltrahandWorldSubsystem* UltrahandSubsystem = nullptr;

	// 잡은 시점의 오브젝트 중심과 충돌 지점 사이의 로컬 오프셋
	FVector GrabLocalOffset;

	// 초기 잡기 시 거리 차이를 보정하기 위한 값
	float TargetGrabDistance = 0.0f;
	float CurrentGrabDistance = 0.0f;

	TObjectPtr<AZCActor> TargetedActor = nullptr;
};
