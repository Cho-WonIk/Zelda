
// Zelda Physics Gear에서 사용하는 시뮬레이션 기어 타입 열거형 정의

#pragma once

#include "CoreTypes.h"

// 기어 상태값
enum class EZCGearState : uint8
{
	Disabled,
	Enabled
};

// 모듈 물리 시뮬레이션 방식
enum class EZCSimGearPhysiscType : uint8
{
	NoPhysics,			// 어떠한 물리 연산도 하지 않음
	EnginePhysics,		// 엔진 내장 물리 연산 사용
	CustomPhysics		// 커스텀 물리 연산 + 엔진 물리 연산 사용
};
