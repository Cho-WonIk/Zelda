// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ZCCanGotoNextSectionNotify.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "ZC다음몽타주섹션이동", ToolTip = "다음 몽타주 섹션으로 이동할 수 있음을 알림, 이동할지 말지는 해당 액터에서 결정"))
class ZELDA_API UZCCanGotoNextSectionNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual FString GetNotifyName_Implementation() const override { return TEXT("ZC다음몽타주섹션이동"); }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	FName MontageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	FName NextSectionName;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
