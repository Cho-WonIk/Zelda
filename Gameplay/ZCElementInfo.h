// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZCElementInfo.generated.h"

USTRUCT(BlueprintType)
struct FElementInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소"))
	FGameplayTag ElementTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "공격자 원소의 지속시간"))
	float Duration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "공격자 원소의 확산 카운트"))
	int32 SpreadCount = -1;

	FElementInfo() {}

	FElementInfo(FGameplayTag InElementTag) : ElementTag(InElementTag) {}

	FElementInfo(FGameplayTag InElementTag, float InDuration, int32 InSpreadCount)
	{
		ElementTag = InElementTag;
		Duration = InDuration;
		SpreadCount = InSpreadCount;
	}

	[[nodiscard]] const bool IsEmpty() const { return ElementTag == FGameplayTag::EmptyTag; }
	[[nodiscard]] const bool IsNewElement() const { return Duration == -1.0f && SpreadCount == -1; }
};
