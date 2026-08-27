// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Subsystem/ZCUltrahandWorldSubsystem.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

#include "Settings/Physics/ZCPhysicsSettings.h"
#include "Development/ZCLogger.h"
#include "Actor/ZCActor.h"
#include "Actor/Util/ZCUtilActor.h"
#include "Actor/Item/Gear/ZCGearActor.h"
#include "ZeldaPhysics/Public/Runtime/Gear/ZCGearMovementComponent.h"
#include "ZeldaPhysics/Public/ZCSimGearManager.h"

FZCConstraintEdge::FZCConstraintEdge(AZCActor* InNodeA, AZCActor* InNodeB, AZCGlueActor* InGlueActor) : NodeA(InNodeA), NodeB(InNodeB), GlueActor(InGlueActor)
{
}

FZCConstraintEdge::~FZCConstraintEdge()
{
	if (ConstraintInstance.IsValidConstraintInstance())
	{
		ConstraintInstance.TermConstraint();
	}
	GlueActor->Destroy();
	//UZCLogger::Warning(TEXT("{0} <-> {1} Connection Removed"), NodeA, NodeB);
}

void UZCUltrahandWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PhysicsConstraintGraph.Empty();
}

void UZCUltrahandWorldSubsystem::PostInitialize()
{
	Super::PostInitialize();

	DefaultPhysicsSetting = GetDefault<UZCPhysicsSettings>();

	if (UWorld* World = GetWorld())
	{
		if (FPhysScene* PhysScene = World->GetPhysicsScene())
		{
			GearManager = FZCSimGearManager::GetManagerFromScene(PhysScene);

			UZCLogger::Warning(TEXT("UltrahandWorldSubsystem initialized. GearManager: {0}"), GearManager);
		}
	}
}

void UZCUltrahandWorldSubsystem::Deinitialize()
{
	PhysicsConstraintGraph.Empty();
	GearManager = nullptr;
	Super::Deinitialize();
}

void UZCUltrahandWorldSubsystem::ConnectActors(AZCActor* ActorA, AZCActor* ActorB, UPrimitiveComponent* ComponentA, UPrimitiveComponent* ComponentB, AZCGlueActor* GlueActor)
{
	if (!ActorA || !ActorB || ActorA == ActorB) return;

	TWeakObjectPtr<AZCActor> WeakA = ActorA;
	TWeakObjectPtr<AZCActor> WeakB = ActorB;

	// 이미 연결되어 있다면 리턴
	if (PhysicsConstraintGraph.Contains(WeakA) && PhysicsConstraintGraph[WeakA].Contains(WeakB)) return;

	// 이미 간접적으로 연결되어 있다면 리턴
	if (GetConnectedGroupSet(ActorA).Contains(ActorB)) return;

	ActorA->OnDestroyed.AddUniqueDynamic(this, &UZCUltrahandWorldSubsystem::OnActorDestroyed);
	ActorB->OnDestroyed.AddUniqueDynamic(this, &UZCUltrahandWorldSubsystem::OnActorDestroyed);

	TSharedPtr<FZCConstraintEdge> NewEdge = MakeShared<FZCConstraintEdge>(ActorA, ActorB, GlueActor);

	// 물리 제약 초기화
	InitConstraint(NewEdge->ConstraintInstance, ActorA, ActorB, ComponentA, ComponentB);

	// 양방향 그래프 등록
	PhysicsConstraintGraph.FindOrAdd(WeakA).Add(WeakB, NewEdge);
	PhysicsConstraintGraph.FindOrAdd(WeakB).Add(WeakA, NewEdge);

	//UZCLogger::Warning(TEXT("Connected {0} <-> {1}"), ActorA, ActorB);
}

void UZCUltrahandWorldSubsystem::DetachActor(AZCActor* TargetActor)
{
	if (!TargetActor) return;

	TWeakObjectPtr<AZCActor> TargetWeak = TargetActor;
	if (!PhysicsConstraintGraph.Contains(TargetWeak)) return;

	TargetActor->OnDestroyed.RemoveDynamic(this, &UZCUltrahandWorldSubsystem::OnActorDestroyed);

	TMap<TWeakObjectPtr<AZCActor>, TSharedPtr<FZCConstraintEdge>>& Neighbors = PhysicsConstraintGraph[TargetWeak];
	TArray<TWeakObjectPtr<AZCActor>> NeighborKeys;
	Neighbors.GetKeys(NeighborKeys);

	for (const auto& NeighborWeak : NeighborKeys)
	{
		if (PhysicsConstraintGraph.Contains(NeighborWeak))
		{
			PhysicsConstraintGraph[NeighborWeak].Remove(TargetWeak);

			if (PhysicsConstraintGraph[NeighborWeak].Num() == 0)
			{
				PhysicsConstraintGraph.Remove(NeighborWeak);
				if (AZCActor* NeighborActor = NeighborWeak.Get())
				{
					NeighborActor->OnDestroyed.RemoveDynamic(this, &UZCUltrahandWorldSubsystem::OnActorDestroyed);
				}
			}
		}
	}

	// 타겟 액터 항목 완전 제거 (여기서 Edge의 SharedPtr 레퍼런스 카운트가 0이 되며 물리 제약 해제됨)
	PhysicsConstraintGraph.Remove(TargetWeak);

	//UZCLogger::Warning(TEXT("Detached {0}"), TargetActor);
}

TSet<AZCActor*> UZCUltrahandWorldSubsystem::GetConnectedGroupSet(AZCActor* TargetActor)
{
	TSet<AZCActor*> Result;
	if (!TargetActor) return Result;

	TQueue<AZCActor*> Queue;
	TSet<AZCActor*> Visited;

	Queue.Enqueue(TargetActor);
	Visited.Add(TargetActor);

	AZCActor* CurrentActor = nullptr;
	while (Queue.Dequeue(CurrentActor))
	{
		// 현재 노드의 이웃들을 그래프에서 찾음
		if (const auto* Neighbors = PhysicsConstraintGraph.Find(CurrentActor))
		{
			for (const auto& Pair : *Neighbors)
			{
				// TWeakObjectPtr이 유효한지 확인
				if (AZCActor* NeighborActor = Pair.Key.Get())
				{
					// 아직 방문하지 않은 액터라면 큐에 삽입
					if (!Visited.Contains(NeighborActor))
					{
						Visited.Add(NeighborActor);
						Queue.Enqueue(NeighborActor);
					}
				}
			}
		}
	}
	return Visited;
}

TSet<AZCGearActor*> UZCUltrahandWorldSubsystem::ActivateConnectedGears(AZCActor* TargetActor)
{
	TSet<AZCGearActor*> GearSet = GetConnectedGroupSet<AZCGearActor>(TargetActor);

	for (auto &Gear : GearSet)
	{
		GearManager->SetGearSimulationEnabled(Gear->GetGearMovementComponent(), true);
	}

	return GearSet;
}

TSet<AZCGearActor*> UZCUltrahandWorldSubsystem::DeactivateConnectedGears(AZCActor* TargetActor)
{
	TSet<AZCGearActor*> GearSet = GetConnectedGroupSet<AZCGearActor>(TargetActor);
	for (auto& Gear : GearSet)
	{
		GearManager->SetGearSimulationEnabled(Gear->GetGearMovementComponent(), false);
	}

	return GearSet;
}

void UZCUltrahandWorldSubsystem::InitConstraint(FConstraintInstance& Constraint, AZCActor* ActorA, AZCActor* ActorB, UPrimitiveComponent* ComponentA, UPrimitiveComponent* ComponentB)
{
	UPrimitiveComponent* RootA = ComponentA ? ComponentA : ActorA->GetMesh();
	UPrimitiveComponent* RootB = ComponentB ? ComponentB : ActorB->GetMesh();

	if (!RootA || !RootB) return;

	float AngularLimit = DefaultPhysicsSetting->MaxAngularLimit;

	Constraint.SetLinearXLimit(LCM_Locked, 0.0f);
	Constraint.SetLinearYLimit(LCM_Locked, 0.0f);
	Constraint.SetLinearZLimit(LCM_Locked, 0.0f);

	Constraint.SetAngularSwing1Limit(ACM_Limited, AngularLimit);
	Constraint.SetAngularSwing2Limit(ACM_Limited, AngularLimit);
	Constraint.SetAngularTwistLimit(ACM_Limited, AngularLimit);

	// 1-1. 선형(Linear) 드라이브 활성화 (X, Y, Z 축 모두)
	Constraint.SetLinearPositionDrive(true, true, true);
	// PositionStrength: 스프링 강도 (P)
	// VelocityStrength: 감쇠력 (D)
	// ForceLimit: 가할 수 있는 최대 힘 (0.0f는 보통 무제한을 의미하거나 엔진 설정에 따름)
	Constraint.SetLinearDriveParams(DefaultPhysicsSetting->LinearPositionStrength, DefaultPhysicsSetting->LinearVelocityStrength, 0.0f);

	// 2-1. 회전(Angular) 드라이브 활성화 (Twist, Swing 모두)
	Constraint.SetOrientationDriveTwistAndSwing(true, true);

	// 2-2. 회전 드라이브 파라미터 적용
	Constraint.SetAngularDriveParams(DefaultPhysicsSetting->AngularPositionStrength, DefaultPhysicsSetting->AngularVelocityStrength, 0.0f);

	FTransform WorldTransformA = RootA->GetComponentTransform();
	FTransform WorldTransformB = RootB->GetComponentTransform();

	FVector MidLocation = (WorldTransformA.GetLocation() + WorldTransformB.GetLocation()) * 0.5f;

	FTransform ConstraintFrameWorld;
	ConstraintFrameWorld.SetLocation(MidLocation);
	ConstraintFrameWorld.SetRotation(WorldTransformA.GetRotation());
	ConstraintFrameWorld.SetScale3D(FVector::OneVector);

	FTransform Frame1 = ConstraintFrameWorld.GetRelativeTransform(WorldTransformA);
	FTransform Frame2 = ConstraintFrameWorld.GetRelativeTransform(WorldTransformB);

	Constraint.Pos1 = Frame1.GetLocation() * WorldTransformA.GetScale3D();
	Constraint.PriAxis1 = Frame1.GetUnitAxis(EAxis::X);
	Constraint.SecAxis1 = Frame1.GetUnitAxis(EAxis::Y);

	Constraint.Pos2 = Frame2.GetLocation() * WorldTransformB.GetScale3D();
	Constraint.PriAxis2 = Frame2.GetUnitAxis(EAxis::X);
	Constraint.SecAxis2 = Frame2.GetUnitAxis(EAxis::Y);

	FBodyInstance* BodyA = RootA->GetBodyInstance();
	FBodyInstance* BodyB = RootB->GetBodyInstance();

	if (BodyA && BodyB)
	{
		TWeakObjectPtr<AZCActor> WeakA = ActorA;
		TWeakObjectPtr<AZCActor> WeakB = ActorB;
		TWeakObjectPtr<UZCUltrahandWorldSubsystem> WeakThis = this;

		Constraint.SetDisableCollision(true);

		// 선형 파괴 (잡아 당기는 힘에 의해 끊어짐
		Constraint.ProfileInstance.bLinearBreakable = true;
		Constraint.ProfileInstance.LinearBreakThreshold = DefaultPhysicsSetting->LinearBreakThreshold;

		// 회전 파괴 (비트는 힘에 의해 끊어짐)
		Constraint.ProfileInstance.bAngularBreakable = true;
		Constraint.ProfileInstance.AngularBreakThreshold = DefaultPhysicsSetting->AngularBreakThreshold;

		// 외력에 의해 끊어졌을 때의 콜백
		Constraint.InitConstraint(BodyA, BodyB, 1.0f, this, FOnConstraintBroken::CreateLambda(
			[WeakThis, WeakA, WeakB](int32 ConstraintIndex)
			{
				AsyncTask(ENamedThreads::GameThread, [WeakThis, WeakA, WeakB]()
					{
						UZCUltrahandWorldSubsystem* Subsystem = WeakThis.Get();
						if (!Subsystem) return;

						if (WeakA.IsValid() && WeakB.IsValid())
						{
							Subsystem->DisconnectActors(WeakA, WeakB);
						}

					});
			}
		));

	}
}

void UZCUltrahandWorldSubsystem::DisconnectActors(TWeakObjectPtr<AZCActor> ActorA, TWeakObjectPtr<AZCActor> ActorB)
{
	// A -> B 연결 제거
	if (PhysicsConstraintGraph.Contains(ActorA))
	{
		PhysicsConstraintGraph[ActorA].Remove(ActorB);

		if (PhysicsConstraintGraph[ActorA].Num() == 0)
		{
			PhysicsConstraintGraph.Remove(ActorA);
			ActorA->OnDestroyed.RemoveDynamic(this, &UZCUltrahandWorldSubsystem::OnActorDestroyed);
		}
	}

	// B -> A 연결 제거
	if (PhysicsConstraintGraph.Contains(ActorB))
	{
		PhysicsConstraintGraph[ActorB].Remove(ActorA);

		if (PhysicsConstraintGraph[ActorB].Num() == 0)
		{
			PhysicsConstraintGraph.Remove(ActorB);
			ActorB->OnDestroyed.RemoveDynamic(this, &UZCUltrahandWorldSubsystem::OnActorDestroyed);
		}
	}
}

void UZCUltrahandWorldSubsystem::OnActorDestroyed(AActor* DestroyedActor)
{
	if (AZCActor* ZCActor = Cast<AZCActor>(DestroyedActor))
	{
		DetachActor(ZCActor);
	}
}
