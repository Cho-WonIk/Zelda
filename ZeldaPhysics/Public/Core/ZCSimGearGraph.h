// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/ZCSimGearEnum.h"

class UZCGearMovementComponent;

class FZCSimGearManager;

namespace ZCGear
{
	class ZELDAPHYSICS_API FZCSimGearGraphBuffer final
	{
	public:
		enum class EOpType
		{
			Add, Remove, ChangeState, ControlPlayerInput, UnControlPlayerInput
		};

		struct FOp
		{
			EOpType Type;
			TWeakObjectPtr<UZCGearMovementComponent> CompA;
			EZCGearState NewState;
		};

	public:
		FZCSimGearGraphBuffer();
		~FZCSimGearGraphBuffer() = default;

		void AddNode(UZCGearMovementComponent* NewNode);
		void RemoveNode(UZCGearMovementComponent* Node);

		void ChangeNodeState(UZCGearMovementComponent* Node, EZCGearState NewState);

		// 플레이어의 입력을 받는 기어 컴포넌트 등록
		void RegisterApplyPlayerInputGear(UZCGearMovementComponent* Node);
		// 플레이어의 입력을 받는 기어 컴포넌트 등록 해제
		void UnRegisterApplyPlayerInputGear(UZCGearMovementComponent* Node);

		bool HasPendingOps() const { return PendingOps.Num() > 0; }

	private:
		friend class FZCSimGearGraph;
		TArray<FOp> PendingOps;
	};

	struct FZCSimGearGraphNode
	{
		UZCGearMovementComponent* GearComponent;
		bool IsValid() const { return GearComponent != nullptr; }
	};

	class ZELDAPHYSICS_API FZCSimGearGraph final
	{
	public:
		FZCSimGearGraph();
		~FZCSimGearGraph();

		// --- 인터페이스 ---
		void ApplyBuffer(class FZCSimGearGraphBuffer& Buffer);
		void TraverseActiveNodes(TFunctionRef<void(UZCGearMovementComponent*)> Func) const;
		const TArray<UZCGearMovementComponent*>& GetCachedActiveGears() { return CachedActiveGears; }
		const TArray<UZCGearMovementComponent*>& GetApplyPlayerInputGears() { return ApplyPlayerInputGears; }

	protected:
		// --- 내부 그래프 조작 ---
		void AddNode(UZCGearMovementComponent* NewNode);
		void RemoveNode(UZCGearMovementComponent* Node);
		void ChangeNodeState(UZCGearMovementComponent* Node, EZCGearState NewState);

		void RebuildCachedActiveGears(UZCGearMovementComponent* Node, bool bIsSimulation);

		// 플레이어의 입력을 받는 기어 컴포넌트 등록
		void RegisterApplyPlayerInputGear(UZCGearMovementComponent* Node);
		// 플레이어의 입력을 받는 기어 컴포넌트 등록 해제
		void UnRegisterApplyPlayerInputGear(UZCGearMovementComponent* Node);


	private:
		// 모든 그래프 노드 목록
		TArray<FZCSimGearGraphNode> Nodes;

		// 활성화된 기어 컴포넌트 캐시 목록, 순서 정렬 X
		TArray<UZCGearMovementComponent*> CachedActiveGears;
		
		// 플레이어의 입력을 받는 기어 컴포넌트 목록, 순서 정렬 X
		TArray<UZCGearMovementComponent*> ApplyPlayerInputGears;
	};

}

