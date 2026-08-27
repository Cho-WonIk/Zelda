// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/Notify/ZCEquipmentNotify.h"
#include "Interface/ZCMontageInterface.h"

void UZCEquipmentNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (MeshComp)
	{
		if (IZCMontageInterface* InterfacePawn = Cast<IZCMontageInterface>(MeshComp->GetOwner()))
		{
			if (bIsDraw)
			{
				InterfacePawn->DrawEquipmentNotify(ItemType);
			}
			else
			{
				InterfacePawn->SheathEquipmentNotify(ItemType);
			}
		}
	}
}
