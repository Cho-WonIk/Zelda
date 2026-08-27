// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ZCPerceptionConfig.generated.h"

USTRUCT(BlueprintType)
struct FSenseDetectionInfo
{
	GENERATED_BODY()
	
	// 감지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (DisplayName = "감지 여부"))
	uint8 bIsDetected : 1;
	// 감지된 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (DisplayName = "감지 위치"))
	FVector SensedLocation;
	// 감지된 액터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (DisplayName = "감지 액터"))
	TWeakObjectPtr<class AActor> SensedActor;
	// 감지된 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (DisplayName = "감지된 시간"))
	float SensedTime;

	FSenseDetectionInfo() : bIsDetected(false), SensedLocation(FVector::ZeroVector), SensedActor(nullptr), SensedTime(-1.0f) {}
};

USTRUCT(BlueprintType)
struct FPerceptionData
{
	GENERATED_BODY()

	// 시각 감지 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception|Sight", meta = (DisplayName = "시각 정보"))
	FSenseDetectionInfo SightInfo;

	// 청각 감지 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception|Hearing", meta = (DisplayName = "청각 정보"))
	FSenseDetectionInfo HearingInfo;

	// 데미지 감지 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception|Damage", meta = (DisplayName = "데미지 정보"))
	FSenseDetectionInfo DamageInfo;

	// 시각에 감지된 액터 중 가장 가까운 액터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception|Sight", meta = (DisplayName = "가장 가까운 시각 액터"))
	TWeakObjectPtr<AActor> ClosestSightActor = nullptr;
};

USTRUCT(BlueprintType)
struct FSenseConfig
{
	GENERATED_BODY()

	// 감각의 최대 수명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "최대 수명"))
	float MaxLifeTime = 5.0f;
	// 감지 데이터의 증가값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "증가값"))
	float AggroIncrement = 0.0f;
	// 감지 데이터의 감소값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "감소값"))
	float AggroDecrement = 0.0f;
};

USTRUCT(BlueprintType)
struct FSightConfig : public FSenseConfig
{
	GENERATED_BODY()

};

USTRUCT(BlueprintType)
struct FHearingConfig : public FSenseConfig
{
	GENERATED_BODY()

};

USTRUCT(BlueprintType)
struct FDamageConfig : public FSenseConfig
{
	GENERATED_BODY()

};

USTRUCT(BlueprintType)
struct FZCPerceptionConfig
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Sight", meta = (DisplayName = "시각 설정"))
	FSightConfig SightConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Hearing", meta = (DisplayName = "청각 설정"))
	FHearingConfig HearingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Damage", meta = (DisplayName = "데미지 설정"))
	FDamageConfig DamageConfig;
};
