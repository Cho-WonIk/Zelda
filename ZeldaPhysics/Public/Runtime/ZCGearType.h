// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreTypes.h"
#include "UObject/ObjectMacros.h"
#include "ZCGearType.generated.h"

UENUM(BlueprintType)
enum class EZCGearType : uint8
{
	Undefined	= 0				UMETA(DisplayName = "미정의"),
	// 자체 시뮬레이션 없음
	Chassis		= 1				UMETA(DisplayName = "차체"),
	//힘을 가함
	Thruster	= 2				UMETA(DisplayName = "추진기"),
	// 항력과 양력 적용
	Aerofoil	= 3				UMETA(DisplayName = "에어포일"),
	// 바퀴
	Wheel		= 4				UMETA(DisplayName = "바퀴"),
	// 입력
	Input		= 5				UMETA(DisplayName = "입력"),
	// 선풍기
	Fan			= 6				UMETA(DisplayName = "선풍기"),
	// 풍선
	Balloon		= 7				UMETA(DisplayName = "풍선"),
	// 서스펜션
	Suspension = 8				UMETA(DisplayName = "서스펜션"),

	Max							UMETA(Hidden),
};

UENUM(BlueprintType)
enum class EZCGearAxisType : uint8
{
	None = 0						UMETA(DisplayName = "없음"),

	XAxis							UMETA(DisplayName = "X축"),
	YAxis							UMETA(DisplayName = "Y축"),
	ZAxis							UMETA(DisplayName = "Z축"),

	Pitch							UMETA(DisplayName = "피치"),
	Yaw								UMETA(DisplayName = "요"),
	Roll							UMETA(DisplayName = "롤"),
	
	Max								UMETA(Hidden),
};

// 적용 방식 : 더하기, 곱하기, 덮어쓰기
UENUM(BlueprintType)
enum class EZCGearInputApplyType : uint8
{
	None = 0						UMETA(DisplayName = "없음"),
	Additive						UMETA(DisplayName = "더하기"),
	Multiplicative					UMETA(DisplayName = "곱하기"),
	Set								UMETA(DisplayName = "덮어쓰기"),
};

// 값 변환 방식, CustomInterpolation인 경우 UCurveFloat을 참조할 수 있어야함
// 선형보간 y = ax + b
// 제곱보간 y = ax^2 + b
// 커스텀 보간 y = af(x) + b
UENUM(BlueprintType)
enum class EZCGearInputModifierType : uint8
{
	// 없음
	None = 0						UMETA(DisplayName = "없음"),
	// 선형 보간 y = ax + b
	LinearInterpolation				UMETA(DisplayName = "선형 보간"),
	// 제곱 보간 y = ax^2 + b
	SquaredInterpolation			UMETA(DisplayName = "제곱 보간"),
	// 커스텀 보간 y = af(x) + b
	CustomInterpolation				UMETA(DisplayName = "커스텀 보간"),
};

// 기어가 입력 벡터의 Y축 방향과 동기화 여부
UENUM(BlueprintType)
enum class EZCGearInputSyncType : uint8
{
	// 입력 벡터와 동기화되지 않음
	None = 0						UMETA(DisplayName = "없음"),
	// 입력 벡터의 Y축 방향에 맞춰 동기화
	Sync							UMETA(DisplayName = "동기화"),
};
