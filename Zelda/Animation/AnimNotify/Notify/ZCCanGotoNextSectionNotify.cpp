// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/Notify/ZCCanGotoNextSectionNotify.h"
#include "Interface/ZCMontageInterface.h"

void UZCCanGotoNextSectionNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (MeshComp)
	{
		if (IZCMontageInterface* MontagePawn = Cast<IZCMontageInterface>(MeshComp->GetOwner()))
		{
			MontagePawn->CanGotoNextSection(MontageName, NextSectionName);
		}
	}
}
