// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ZCCombatNotifyUtils.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "ZC공격판정시간", ToolTip = "트레이스 채널로 공격 판정하는 시간"))
class ZELDA_API UZCHitTraceNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual FString GetNotifyName_Implementation() const override { return TEXT("ZC공격판정"); }

public:
	// 다단 히트 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify", meta = (DisplayName = "다단 히트 여부"))
	bool bAllowMultipleHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify", meta = (DisplayName = "다단 히트 간격"))
	float HitInterval = 0.0f;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference);
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);
};

UCLASS(meta = (DisplayName = "ZC패리판정시간", ToolTip = "패리 판정이 나는 시간"))
class ZELDA_API UZCParryNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual FString GetNotifyName_Implementation() const override { return TEXT("ZC패리판정"); }

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference);
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);
};