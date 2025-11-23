// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ZCActor.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "ZCGearActor.generated.h"

class UPrimitiveComponent;
struct FBodyInstance;

// 여러 조인트를 표현할 인덱싱 구조체
USTRUCT()
struct FIndexedConstraintJoint
{
	GENERATED_BODY()

	// ConstraintInstances 배열에 대한 인덱스
	UPROPERTY()
	uint16 ConstraintIndex = 0;

	// ComponentTable 배열 인덱스
	UPROPERTY()
	uint16 Component1Index = 0;
	UPROPERTY()
	uint16 Component2Index = 0;

	// BoneNameTable 배열 인덱스
	UPROPERTY()
	uint16 BoneName1Index = 0;
	UPROPERTY()
	uint16 BoneName2Index = 0;

	// 시스템 내부 관리용 플래그 (생성 여부 등)
	UPROPERTY()
	uint8 bInitialized : 1;

	FIndexedConstraintJoint()
		: ConstraintIndex(0)
		, Component1Index(0)
		, Component2Index(0)
		, BoneName1Index(0)
		, BoneName2Index(0)
		, bInitialized(false)
	{
	}
};

/** 울트라 핸드로 아이템을 조합해 얻게 되는 액터들의 집합, 피지컬 조인트 관리용 액터
 * 
 */
UCLASS()
class ZELDA_API AZCGearActor : public AZCActor
{
	GENERATED_BODY()
	
public:
	AZCGearActor(const FObjectInitializer& ObjectInitializer);

protected:
	// AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ==== 인덱싱 버퍼 ====

	// 여러 조인트에서 공유하는 컴포넌트 테이블
	UPROPERTY()
	TArray<TWeakObjectPtr<UPrimitiveComponent>> ComponentTable;

	// 여러 조인트에서 공유하는 BoneName 테이블
	UPROPERTY()
	TArray<FName> BoneNameTable;

	// 실제 물리 조인트 인스턴스들
	UPROPERTY()
	TArray<FConstraintInstance> ConstraintInstances;

	// 각각의 조인트를 인덱스로 표현
	UPROPERTY(EditAnywhere, Category = "Physics|Constraints")
	TArray<FIndexedConstraintJoint> Joints;

public:
	// ===== 조인트 생성/삭제 =====

	/** 새 조인트 추가, JointHandle(인덱스) 반환 */
	int32 AddConstraint(UPrimitiveComponent* Component1, FName BoneName1, UPrimitiveComponent* Component2, FName BoneName2, const FConstraintInstance& TemplateInstance);

	/** 조인트 완전 제거 (물리 + 데이터) */
	void RemoveConstraint(int32 JointHandle);

	/** 조인트 끊기 (물리만 끊고 데이터 유지 가능) */
	void BreakConstraint(int32 JointHandle, bool bKeepData = true);

	/** 모든 조인트 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints")
	void InitAllConstraints();

	/** 모든 조인트 종료 */
	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints")
	void TermAllConstraints();

	/** 조인트에 연결된 컴포넌트/본 정보 얻기 */
	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints")
	bool GetConstraintInfo(int32 JointHandle, UPrimitiveComponent*& OutComp1, FName& OutBone1, UPrimitiveComponent*& OutComp2, FName& OutBone2) const;

	/** 조인트에 작용한 힘 얻기 */
	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints")
	bool GetConstraintForce(int32 JointHandle, FVector& OutLinearForce, FVector& OutAngularForce);

	/** 조인트가 깨졌는지 여부 */
	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints")
	bool IsConstraintBroken(int32 JointHandle);

	// ===== Limit 계열 =====

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Limit")
	void SetLinearXLimit(int32 JointHandle, ELinearConstraintMotion Motion, float LinearLimit);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Limit")
	void SetLinearYLimit(int32 JointHandle, ELinearConstraintMotion Motion, float LinearLimit);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Limit")
	void SetLinearZLimit(int32 JointHandle, ELinearConstraintMotion Motion, float LinearLimit);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Limit")
	void SetAngularSwing1Limit(int32 JointHandle, EAngularConstraintMotion Motion, float Swing1LimitAngle);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Limit")
	void SetAngularSwing2Limit(int32 JointHandle, EAngularConstraintMotion Motion, float Swing2LimitAngle);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Limit")
	void SetAngularTwistLimit(int32 JointHandle, EAngularConstraintMotion Motion, float TwistLimitAngle);

	// ===== Breakable / Plasticity / ContactTransfer =====

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Break")
	void SetLinearBreakable(int32 JointHandle, bool bLinearBreakable, float LinearBreakThreshold);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Plasticity")
	void SetLinearPlasticity(int32 JointHandle, bool bLinearPlasticity, float LinearPlasticityThreshold, EConstraintPlasticityType PlasticityType);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Break")
	void SetAngularBreakable(int32 JointHandle, bool bAngularBreakable, float AngularBreakThreshold);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Plasticity")
	void SetAngularPlasticity(int32 JointHandle, bool bAngularPlasticity, float AngularPlasticityThreshold);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Contact")
	void SetContactTransferScale(int32 JointHandle, float ContactTransferScale);

	// ===== Projection =====

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Projection")
	void SetProjectionEnabled(int32 JointHandle, bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Projection")
	void SetProjectionParams(int32 JointHandle, float ProjectionLinearAlpha, float ProjectionAngularAlpha, float ProjectionLinearTolerance, float ProjectionAngularTolerance);

	// ===== Linear Drive 계열 =====

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetLinearPositionDrive(int32 JointHandle, bool bEnableDriveX, bool bEnableDriveY, bool bEnableDriveZ);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetLinearVelocityDrive(int32 JointHandle, bool bEnableDriveX, bool bEnableDriveY, bool bEnableDriveZ);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetLinearPositionTarget(int32 JointHandle, const FVector& InPosTarget);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetLinearVelocityTarget(int32 JointHandle, const FVector& InVelTarget);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetLinearDriveParams(int32 JointHandle, float PositionStrength, float VelocityStrength, float InForceLimit);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetLinearDriveAccelerationMode(int32 JointHandle, bool bAccelerationMode);

	// ===== Angular Drive 계열 =====

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetOrientationDriveTwistAndSwing(int32 JointHandle, bool bEnableTwistDrive, bool bEnableSwingDrive);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetOrientationDriveSLERP(int32 JointHandle, bool bEnableSLERP);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetAngularVelocityDriveTwistAndSwing(int32 JointHandle, bool bEnableTwistDrive, bool bEnableSwingDrive);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetAngularVelocityDriveSLERP(int32 JointHandle, bool bEnableSLERP);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetAngularDriveMode(int32 JointHandle, EAngularDriveMode::Type DriveMode);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetAngularDriveAccelerationMode(int32 JointHandle, bool bAccelerationMode);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetAngularOrientationTarget(int32 JointHandle, const FRotator& InPosTarget);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetAngularVelocityTarget(int32 JointHandle, const FVector& InVelTarget);

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|Drive")
	void SetAngularDriveParams(int32 JointHandle, float PositionStrength, float VelocityStrength, float InForceLimit);

	// ===== 현재 각도 조회 =====

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|State")
	float GetCurrentTwist(int32 JointHandle) const;

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|State")
	float GetCurrentSwing1(int32 JointHandle) const;

	UFUNCTION(BlueprintCallable, Category = "Physics|Constraints|State")
	float GetCurrentSwing2(int32 JointHandle) const;

protected:
	// ===== 인덱싱 버퍼 유틸 =====
	int32 FindOrAddComponent(UPrimitiveComponent* Component);
	int32 FindOrAddBoneName(FName BoneName);

	FConstraintInstance* GetConstraintInstance(int32 JointHandle);
	const FConstraintInstance* GetConstraintInstance(int32 JointHandle) const;

	FBodyInstance* GetBodyInstance(int32 JointHandle, bool bFrame1) const;
	Chaos::FPhysicsObject* GetPhysicsObject(int32 JointHandle, bool bFrame1) const;

	void InitConstraintInternal(int32 JointHandle);
	void TermConstraintInternal(int32 JointHandle);
};
