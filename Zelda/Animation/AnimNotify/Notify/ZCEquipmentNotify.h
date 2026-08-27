// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameData/Enum/ZCItemType.h"
#include "ZCEquipmentNotify.generated.h"


UCLASS(meta = (DisplayName = "ZC장착 및 해제", ToolTip = "애니메이션의 타이밍에 맞추어 장비를 장착및 해제"))
class ZELDA_API UZCEquipmentNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override { return TEXT("ZC장착 및 해제"); }
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	EItemType ItemType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	bool bIsDraw; // true: 장착, false: 해제

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
