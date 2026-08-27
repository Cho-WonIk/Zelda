// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "ZCUtilActor.generated.h"

// 사용 안하는 메타볼 액터
UCLASS()
class ZELDA_API AZCMetaBallActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZCMetaBallActor();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> StaticMesh;
};

// 접착 지점 표시용 액터
UCLASS()
class ZELDA_API AZCGluePointActor : public AActor
{
	GENERATED_BODY()

public:
	AZCGluePointActor();

	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	void SetPosition(const FVector& StartPos, const FVector& EndPos);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> StartMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> EndMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNiagaraComponent> VFXComponent;
};

UCLASS()
class ZELDA_API AZCGizmoActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZCGizmoActor();

protected:
	virtual void PostInitializeComponents() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> YawMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> PitchMesh;

	UPROPERTY(EditAnywhere, Category = "Gizmo|Config")
	FLinearColor DefaultYawColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, Category = "Gizmo|Config")
	FLinearColor DefaultPitchColor = FLinearColor::Green;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> YawDMI;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PitchDMI;

	FName ColorParamName = TEXT("Color");
};

class AZCActor;
class UZCUltrahandWorldSubsystem;

UCLASS()
class ZELDA_API AZCGlueActor : public AActor
{
	GENERATED_BODY()

public:
	AZCGlueActor();

	/**
	 * 물리 시뮬레이션을 끄고, 위치를 보정한 뒤, 결합하고, 다시 물리를 켭니다.
	 * @param WorldSubsystem      : 울트라핸드 서브시스템 (그룹 조회 및 결합용)
	 * @param SourceActor         : 잡고 있는 액터 (이동의 기준)
	 * @param TargetActor         : 붙일 대상 액터 (고정)
	 * @param SourceComponent     : 잡고 있는 액터의 컴포넌트
	 * @param TargetComponent     : 붙일 대상 액터의 컴포넌트
	 * @param SourceSnapLocation  : 잡고 있는 액터의 접착 지점 (World)
	 * @param TargetSnapLocation  : 붙일 대상 액터의 접착 지점 (World)
	 */
	void PerformAttachment(
		UZCUltrahandWorldSubsystem* WorldSubsystem,
		AZCActor* SourceActor,
		AZCActor* TargetActor,
		UPrimitiveComponent* SourceComponent,
		UPrimitiveComponent* TargetComponent,
		const FVector& SourceSnapLocation,
		const FVector& TargetSnapLocation
	);

protected:
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	//virtual void Destroyed() override;

	void ConnectFinished();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> ConnectAnimCurve;

private:
	UZCUltrahandWorldSubsystem* CachedWorldSubsystem = nullptr;

	AZCActor* SourceRootActor = nullptr;
	AZCActor* TargetRootActor = nullptr;

	UPrimitiveComponent* SourceComp = nullptr;
	UPrimitiveComponent* TargetComp = nullptr;

	// 이동시켜야 할 소스 그룹 액터들
	UPROPERTY()
	TSet<AZCActor*> SourceGroupActors;

	// 고정되어 있어야 할 타겟 그룹 액터들
	UPROPERTY()
	TSet<AZCActor*> TargetGroupActors;

	struct FGlueMoveData
	{
		AZCActor* Actor = nullptr;
		FVector InitialLocation = FVector::ZeroVector;
	};

	TArray<FGlueMoveData> MovingActorsData;

	// 이동해야 할 총 거리 벡터 (TargetSnap - SourceSnap)
	FVector TotalMoveOffset = FVector::ZeroVector;

	FVector TargetRelativeAttachLocation = FVector::ZeroVector;

	// 애니메이션 관련
	float MinTime = 0.0f;
	float MaxTime = 1.0f; // 기본값
	float CurrentCurveTime = 0.0f;
};
