// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "ZCUltrahandWorldSubsystem.generated.h"

class UPrimitiveComponent;
struct FBodyInstance;

class AZCActor;
class AZCGearActor;
class AZCGlueActor;
class UZCPhysicsSettings;

struct FZCConstraintEdge
{
	FConstraintInstance ConstraintInstance;

	TWeakObjectPtr<AZCActor> NodeA;
	TWeakObjectPtr<AZCActor> NodeB;

	TStrongObjectPtr<AZCGlueActor> GlueActor;

	FZCConstraintEdge(AZCActor* InNodeA, AZCActor* InNodeB, AZCGlueActor* InGlueActor);

	~FZCConstraintEdge();
};

UCLASS()
class ZELDA_API UZCUltrahandWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void PostInitialize() override;
	virtual void Deinitialize() override;

public:
	// 두 액터를 물리적으로 연결하고 그래프에 등록합니다.
	void ConnectActors(AZCActor* ActorA, AZCActor* ActorB, UPrimitiveComponent* ComponentA, UPrimitiveComponent* ComponentB, AZCGlueActor* GlueActor);

	// 특정 액터를 그래프에서 분리합니다. (연결된 모든 제약 해제)
	void DetachActor(AZCActor* TargetActor);

	// 특정 액터가 속한 연결된 그룹(Island)을 반환합니다.
    TSet<AZCActor*> GetConnectedGroupSet(AZCActor* TargetActor);

	template<typename T>
	TSet<T*> GetConnectedGroupSet(AZCActor* TargetActor);

    // 특정 액터가 속한 그룹의 기어 모듈들을 전부 활성화합니다. 반환값은 활성화되는 기어 액터 목록
    TSet<AZCGearActor*> ActivateConnectedGears(AZCActor* TargetActor);
	// 특정 액터가 속한 그룹의 기어 모듈들을 전부 비활성화합니다. 반환값은 비활성화되는 기어 액터 목록
    TSet<AZCGearActor*> DeactivateConnectedGears(AZCActor* TargetActor);

private:
	// 내부 헬퍼: 제약 조건 설정
	void InitConstraint(FConstraintInstance& Constraint, AZCActor* ActorA, AZCActor* ActorB, UPrimitiveComponent* ComponentA, UPrimitiveComponent* ComponentB);

	// 두 액터간의 연결을 제거
	void DisconnectActors(TWeakObjectPtr<AZCActor> ActorA, TWeakObjectPtr<AZCActor> ActorB);

	// GC 혹은 Destroy() 호출 시 자동으로 그래프 정리용
	UFUNCTION()
	void OnActorDestroyed(AActor* DestroyedActor);

private:
	TMap<TWeakObjectPtr<class AZCActor>, TMap<TWeakObjectPtr<class AZCActor>, TSharedPtr<struct FZCConstraintEdge>>> PhysicsConstraintGraph;

	const UZCPhysicsSettings* DefaultPhysicsSetting = nullptr;

	class FZCSimGearManager* GearManager = nullptr;
};

template<typename T>
TSet<T*> UZCUltrahandWorldSubsystem::GetConnectedGroupSet(AZCActor* TargetActor)
{
    TSet<T*> Result;
    if (!TargetActor) return Result;

    TQueue<AZCActor*> Queue;
    TSet<AZCActor*> Visited;

    Queue.Enqueue(TargetActor);
    Visited.Add(TargetActor);

    // 시작 노드 캐스팅 확인
    if (T* CastedStartNode = Cast<T>(TargetActor))
    {
        Result.Add(CastedStartNode);
    }

    AZCActor* CurrentActor = nullptr;
    while (Queue.Dequeue(CurrentActor))
    {
        if (const auto* Neighbors = PhysicsConstraintGraph.Find(CurrentActor))
        {
            for (const auto& Pair : *Neighbors)
            {
                if (AZCActor* NeighborActor = Pair.Key.Get())
                {
                    if (!Visited.Contains(NeighborActor))
                    {
                        Visited.Add(NeighborActor);
                        Queue.Enqueue(NeighborActor);

                        // 하위 타입으로 캐스팅 가능한 경우에만 결과셋에 추가
                        if (T* CastedActor = Cast<T>(NeighborActor))
                        {
                            Result.Add(CastedActor);
                        }
                    }
                }
            }
        }
    }
    return Result;
}
