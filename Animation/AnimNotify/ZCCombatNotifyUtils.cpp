// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/ZCCombatNotifyUtils.h"
#include "Interface/ZCCombatInterface.h"


// 히트 판정 트레이스
void UZCHitTraceNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp)
	{
		if (IZCCombatInterface* CombatPawn = Cast<IZCCombatInterface>(MeshComp->GetOwner()))
		{
			CombatPawn->OnTraceHit(true, bAllowMultipleHit, HitInterval);
		}
	}
}

void UZCHitTraceNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		if (IZCCombatInterface* CombatPawn = Cast<IZCCombatInterface>(MeshComp->GetOwner()))
		{
			CombatPawn->OnTraceHit(false, bAllowMultipleHit, HitInterval);
		}
	}
}

// 패리 판정
void UZCParryNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp)
	{
		if (IZCCombatInterface* CombatPawn = Cast<IZCCombatInterface>(MeshComp->GetOwner()))
		{
			CombatPawn->OnParry(true);
		}
	}
}

void UZCParryNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp)
	{
		if (IZCCombatInterface* CombatPawn = Cast<IZCCombatInterface>(MeshComp->GetOwner()))
		{
			CombatPawn->OnParry(false);
		}
	}
}
