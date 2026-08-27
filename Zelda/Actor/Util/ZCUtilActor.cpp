// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Util/ZCUtilActor.h"
#include "NiagaraComponent.h"
#include "Actor/ZCActor.h"
#include "World/Subsystem/ZCUltrahandWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCUtilActor)

// Sets default values
AZCMetaBallActor::AZCMetaBallActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = StaticMesh;

	SetActorEnableCollision(false);
}

/////////////////////////////////////////////////////////////////////////////////////////

AZCGluePointActor::AZCGluePointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	StartMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartMesh"));
	EndMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndMesh"));
	
	VFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFXComponent"));

	RootComponent = StartMesh;

	EndMesh->SetupAttachment(RootComponent);
	VFXComponent->SetupAttachment(RootComponent);
}

void AZCGluePointActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (VFXComponent)
	{
		VFXComponent->SetHiddenInGame(bNewHidden);

		if (bNewHidden)
		{
			VFXComponent->Deactivate();
		}
		else
		{
			VFXComponent->Activate(true);
		}
	}
}

void AZCGluePointActor::SetPosition(const FVector& StartPos, const FVector& EndPos)
{
	SetActorLocation(StartPos);
	EndMesh->SetWorldLocation(EndPos);

	VFXComponent->SetNiagaraVariablePosition(TEXT("User.BeamStart"), StartPos);
	VFXComponent->SetNiagaraVariablePosition(TEXT("User.BeamEnd"), EndPos);
}

/////////////////////////////////////////////////////////////////////////////////////////

AZCGizmoActor::AZCGizmoActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	YawMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Yaw"));
	YawMesh->SetupAttachment(RootComponent);

	PitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pitch"));
	PitchMesh->SetupAttachment(RootComponent);


	SetActorEnableCollision(false);
}

void AZCGizmoActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (YawMesh)
	{
		if (YawDMI == nullptr)
		{
			YawDMI = YawMesh->CreateAndSetMaterialInstanceDynamic(0);
		}

		if (YawDMI)
		{
			YawDMI->SetVectorParameterValue(ColorParamName, DefaultYawColor);
		}
	}

	if (PitchMesh)
	{
		if (PitchDMI == nullptr)
		{
			PitchDMI = PitchMesh->CreateAndSetMaterialInstanceDynamic(0);
		}

		if (PitchDMI)
		{
			PitchDMI->SetVectorParameterValue(ColorParamName, DefaultPitchColor);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

AZCGlueActor::AZCGlueActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCanEverAffectNavigation(false);

	MeshComponent->SetVisibility(false);
}

void AZCGlueActor::PerformAttachment(UZCUltrahandWorldSubsystem* WorldSubsystem, AZCActor* SourceActor, AZCActor* TargetActor, UPrimitiveComponent* SourceComponent, UPrimitiveComponent* TargetComponent, const FVector& SourceSnapLocation, const FVector& TargetSnapLocation)
{
	if (!WorldSubsystem || !SourceActor || !TargetActor)
	{
		Destroy();
		return;
	}

	CachedWorldSubsystem = WorldSubsystem;
	SourceRootActor = SourceActor;
	TargetRootActor = TargetActor;
	SourceComp = SourceComponent;
	TargetComp = TargetComponent;

	TargetRelativeAttachLocation = TargetComp->GetComponentTransform().InverseTransformPosition(TargetSnapLocation);

	SourceGroupActors = WorldSubsystem->GetConnectedGroupSet(SourceActor);
	TargetGroupActors = WorldSubsystem->GetConnectedGroupSet(TargetActor);

	MovingActorsData.Reset();
	MovingActorsData.Reserve(SourceGroupActors.Num());

	// 소스 그룹: 초기 위치 저장 및 물리 끄기
	for (AZCActor* Actor : SourceGroupActors)
	{
		MovingActorsData.Add({ Actor, Actor->GetActorLocation() });
		Actor->GetMesh()->SetSimulatePhysics(false);
	}

	// 타겟 그룹: 물리 끄기
	for (AZCActor* Actor : TargetGroupActors)
	{
		Actor->GetMesh()->SetSimulatePhysics(false);
	}

	// 이동 벡터 계산
	TotalMoveOffset = TargetSnapLocation - SourceSnapLocation;

	CurrentCurveTime = MinTime;
	SetActorTickEnabled(true);
}

void AZCGlueActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ConnectAnimCurve)
	{
		ConnectAnimCurve->GetTimeRange(MinTime, MaxTime);
	}
}

void AZCGlueActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentCurveTime += DeltaTime;

	// Alpha(진행률) 계산
	float Alpha = 0.0f;
	if (ConnectAnimCurve)
	{
		Alpha = ConnectAnimCurve->GetFloatValue(CurrentCurveTime);
	}
	else
	{
		Alpha = 1.0f;
		CurrentCurveTime = MaxTime;
	}

	// 소스 그룹 위치 보간
	for (const FGlueMoveData& Data : MovingActorsData)
	{
		FVector NewLoc = Data.InitialLocation + (TotalMoveOffset * Alpha);
		Data.Actor->SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);
	}

	// 종료 체크
	if (CurrentCurveTime >= MaxTime)
	{
		// 최종 위치 강제 동기화
		for (const FGlueMoveData& Data : MovingActorsData)
		{
			Data.Actor->SetActorLocation(Data.InitialLocation + TotalMoveOffset, false, nullptr, ETeleportType::TeleportPhysics);
		}

		ConnectFinished();
	}
}

void AZCGlueActor::ConnectFinished()
{
	SetActorTickEnabled(false);

	// 실제 물리적 결합 수행
	if (CachedWorldSubsystem && SourceRootActor && TargetRootActor)
	{
		CachedWorldSubsystem->ConnectActors(SourceRootActor, TargetRootActor, SourceComp, TargetComp, this);
	}

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
	AttachToComponent(TargetComp, AttachmentRules);

	SetActorRelativeLocation(TargetRelativeAttachLocation);

	MeshComponent->SetVisibility(true);

	// 물리 시뮬레이션 재개
	for (AZCActor* Actor : SourceGroupActors)
	{
		const auto& Root = Actor->GetMesh();

		Root->SetSimulatePhysics(true);
		Root->WakeAllRigidBodies();
	}

	// 물리 시뮬레이션 재개
	for (AZCActor* Actor : TargetGroupActors)
	{
		const auto& Root = Actor->GetMesh();

		Root->SetSimulatePhysics(true);
		Root->WakeAllRigidBodies();
	}

	// 데이터 정리
	SourceGroupActors.Empty();
	TargetGroupActors.Empty();
	MovingActorsData.Empty();

	CachedWorldSubsystem = nullptr;
	SourceRootActor = nullptr;
	TargetRootActor = nullptr;
}
