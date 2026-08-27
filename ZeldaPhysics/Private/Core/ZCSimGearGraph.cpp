// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ZCSimGearGraph.h"
#include "Core/ZCSimGearSimulation.h"

#include "Runtime/Gear/ZCGearMovementComponent.h"

namespace ZCGear
{
	FZCSimGearGraphBuffer::FZCSimGearGraphBuffer()
	{
		PendingOps.Reserve(64);
	}
	void FZCSimGearGraphBuffer::AddNode(UZCGearMovementComponent* NewNode)
	{
		if (!NewNode) return;
		PendingOps.Add({ EOpType::Add, NewNode });
	}

	void FZCSimGearGraphBuffer::RemoveNode(UZCGearMovementComponent* Node)
	{
		if (!Node) return;
		PendingOps.Add({ EOpType::Remove, Node });
	}

	void FZCSimGearGraphBuffer::ChangeNodeState(UZCGearMovementComponent* Node, EZCGearState NewState)
	{
		if (!Node) return;
		PendingOps.Add({ EOpType::ChangeState, Node, NewState });
	}

	void FZCSimGearGraphBuffer::RegisterApplyPlayerInputGear(UZCGearMovementComponent* Node)
	{
		if (!Node) return;
		PendingOps.Add({ EOpType::ControlPlayerInput, Node });
	}

	void FZCSimGearGraphBuffer::UnRegisterApplyPlayerInputGear(UZCGearMovementComponent* Node)
	{
		if (!Node) return;
		PendingOps.Add({ EOpType::UnControlPlayerInput, Node });
	}

	// --------------------------------------------------

	FZCSimGearGraph::FZCSimGearGraph()
	{
		Nodes.Reserve(128);
		CachedActiveGears.Reserve(128);
	}

	FZCSimGearGraph::~FZCSimGearGraph()
	{
	}

	void FZCSimGearGraph::ApplyBuffer(FZCSimGearGraphBuffer& Buffer)
	{
		if (!Buffer.HasPendingOps()) return;

		for (const FZCSimGearGraphBuffer::FOp& Op : Buffer.PendingOps)
		{
			if (!Op.CompA.IsValid()) continue;

			switch (Op.Type)
			{
			case FZCSimGearGraphBuffer::EOpType::Add:
				AddNode(Op.CompA.Get());
				break;
			case FZCSimGearGraphBuffer::EOpType::Remove:
				RemoveNode(Op.CompA.Get());
				break;
			case FZCSimGearGraphBuffer::EOpType::ChangeState:
				ChangeNodeState(Op.CompA.Get(), Op.NewState);
				break;
			case FZCSimGearGraphBuffer::EOpType::ControlPlayerInput:
				RegisterApplyPlayerInputGear(Op.CompA.Get());
				break;
			case FZCSimGearGraphBuffer::EOpType::UnControlPlayerInput:
				UnRegisterApplyPlayerInputGear(Op.CompA.Get());
				break;
			}
		}

		Buffer.PendingOps.Empty();
	}

	void FZCSimGearGraph::TraverseActiveNodes(TFunctionRef<void(UZCGearMovementComponent*)> Func) const
	{
		for (UZCGearMovementComponent* Gear : CachedActiveGears)
		{
			if (Gear)
			{
				Func(Gear);
			}
		}
	}

	void FZCSimGearGraph::AddNode(UZCGearMovementComponent* NewNode)
	{
		if (!NewNode || NewNode->GearTreeNodeIndex != INDEX_NONE) return;

		int32 NewIndex = Nodes.Add({ NewNode });
		NewNode->GearTreeNodeIndex = NewIndex;

		if (NewNode->GearSimulationPT && NewNode->GearSimulationPT->IsSimulating())
		{
			CachedActiveGears.AddUnique(NewNode);
		}
	}

	void FZCSimGearGraph::RemoveNode(UZCGearMovementComponent* Node)
	{
		if (!Node || Node->GearTreeNodeIndex == INDEX_NONE) return;

		int32 Index = Node->GearTreeNodeIndex;
		if (Nodes.IsValidIndex(Index))
		{
			CachedActiveGears.RemoveSingleSwap(Node);
			ApplyPlayerInputGears.RemoveSingleSwap(Node);

			// 마지막 요소와 Swap하여 삭제 (O(1))
			int32 LastIndices = Nodes.Num() - 1;
			if (Index != LastIndices)
			{
				Nodes[Index] = Nodes[LastIndices];
				// 이동된 노드의 인덱스 정보 업데이트
				if (Nodes[Index].GearComponent)
				{
					Nodes[Index].GearComponent->GearTreeNodeIndex = Index;
				}
			}
			Nodes.Pop();
			Node->GearTreeNodeIndex = INDEX_NONE;
		}
	}

	void FZCSimGearGraph::ChangeNodeState(UZCGearMovementComponent* Node, EZCGearState NewState)
	{
		if (!Node || !Nodes.IsValidIndex(Node->GearTreeNodeIndex)) return;

		Node->SetGearState(NewState);

		// 인덱스가 아닌 포인터 기반으로 캐시 갱신
		RebuildCachedActiveGears(Node, Node->GearSimulationPT->IsSimulating());
	}

	void FZCSimGearGraph::RebuildCachedActiveGears(UZCGearMovementComponent* Node, bool bIsSimulation)
	{
		if (!Node) return;

		if (bIsSimulation)
		{
			// 중복 추가 방지
			CachedActiveGears.AddUnique(Node);
		}
		else
		{
			// 해당 객체만 제거
			CachedActiveGears.RemoveSingleSwap(Node);
		}
	}

	void FZCSimGearGraph::RegisterApplyPlayerInputGear(UZCGearMovementComponent* Node)
	{
		if (!Node) return;
		ApplyPlayerInputGears.AddUnique(Node);
		Node->GearSimulationPT->SetAcceptingPlayerInput(true);
	}

	void FZCSimGearGraph::UnRegisterApplyPlayerInputGear(UZCGearMovementComponent* Node)
	{
		if (!Node) return;
		ApplyPlayerInputGears.RemoveSingleSwap(Node);
		Node->GearSimulationPT->SetAcceptingPlayerInput(false);
	}
}

