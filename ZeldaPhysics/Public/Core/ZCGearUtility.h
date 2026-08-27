// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Chaos/Core.h"
#include "Chaos/Real.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "HAL/PlatformCrt.h"
#include "Math/NumericLimits.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"

#define ZC_API ZELDAPHYSICS_API

namespace ZCGear
{
	// 리터럴 네임스페이스 정의
	namespace Literals
	{
		// ==============================================================================
		// 1. 길이 (Length)
		// 언리얼 기준 단위: cm
		// ==============================================================================

		// cm
		FORCEINLINE constexpr float operator"" _cm(long double Val) { return static_cast<float>(Val); }
		// m -> cm
		FORCEINLINE constexpr float operator"" _m(long double Val) { return static_cast<float>(Val * 100.0); }
		// km -> cm
		FORCEINLINE constexpr float operator"" _km(long double Val) { return static_cast<float>(Val * 100000.0); }


		// ==============================================================================
		// 2. 시간 (Time)
		// 언리얼 기준 단위: s (초)
		// ==============================================================================
		FORCEINLINE constexpr float operator"" _sec(long double Val) { return static_cast<float>(Val); }
		FORCEINLINE constexpr float operator"" _min(long double Val) { return static_cast<float>(Val * 60.0); }
		FORCEINLINE constexpr float operator"" _hr(long double Val) { return static_cast<float>(Val * 3600.0); }


		// ==============================================================================
		// 3. 질량 (Mass)
		// 언리얼(Chaos) 기준 단위: kg
		// ==============================================================================
		FORCEINLINE constexpr float operator"" _kg(long double Val) { return static_cast<float>(Val); }
		FORCEINLINE constexpr float operator"" _g(long double Val) { return static_cast<float>(Val * 0.001); }
		FORCEINLINE constexpr float operator"" _ton(long double Val) { return static_cast<float>(Val * 1000.0); }


		// ==============================================================================
		// 4. 속도 (Velocity)
		// 언리얼 기준 단위: cm/s
		// ==============================================================================
		// cm/s -> cm/s
		FORCEINLINE constexpr float operator"" _cm_s(long double Val) { return static_cast<float>(Val); }
		// m/s -> cm/s
		FORCEINLINE constexpr float operator"" _m_s(long double Val) { return static_cast<float>(Val * 100.0); }
		// km/h -> cm/s (KmHToCmS 로직: * 100000 / 3600)
		FORCEINLINE constexpr float operator"" _kmh(long double Val) { return static_cast<float>(Val * 100000.0 / 3600.0); }
		// mph -> cm/s
		FORCEINLINE constexpr float operator"" _mph(long double Val) { return static_cast<float>(Val * 160934.4 / 3600.0); }


		// ==============================================================================
		// 5. 각도 (Angle)
		// 언리얼 기준 단위: Radians
		// ==============================================================================
		// Degree -> Radian
		FORCEINLINE constexpr float operator"" _deg(long double Val) { return static_cast<float>(Val * PI / 180.0); }
		// Radian -> Radian
		FORCEINLINE constexpr float operator"" _rad(long double Val) { return static_cast<float>(Val); }


		// ==============================================================================
		// 6. 각속도 (Angular Velocity)
		// 언리얼 기준 단위: rad/s
		// ==============================================================================
		// RPM -> rad/s (RPMToOmega 로직: * PI / 30)
		FORCEINLINE constexpr float operator"" _rpm(long double Val) { return static_cast<float>(Val * PI / 30.0); }
		// deg/s -> rad/s
		FORCEINLINE constexpr float operator"" _deg_s(long double Val) { return static_cast<float>(Val * PI / 180.0); }


		// ==============================================================================
		// 7. 힘 (Force)
		// 언리얼 기준 단위: kg * cm / s^2 (Centi-Newtons)
		// SI 단위(Newton) = kg * m / s^2 이므로 언리얼 단위로 변환 시 100을 곱해야 함.
		// ==============================================================================
		FORCEINLINE constexpr float operator"" _N(long double Val) { return static_cast<float>(Val * 100.0); }
		// kilonewton -> Unreal Force
		FORCEINLINE constexpr float operator"" _kN(long double Val) { return static_cast<float>(Val * 100000.0); }


		// ==============================================================================
		// 8. 토크 (Torque)
		// 언리얼 기준 단위: kg * cm^2 / s^2
		// SI 단위(Nm) = kg * m^2 / s^2
		// ==============================================================================
		FORCEINLINE constexpr float operator"" _Nm(long double Val) { return static_cast<float>(Val * 10000.0); }	
	}

	class ZC_API FZCDebugDraw
	{
	public:
		static void Line(const FVector& Start, const FVector& End, const FColor& Color = FColor::Red, float Thickness = 2.0f, float LifeTime = -1.0f);
		static void Arrow(const FVector& Start, const FVector& End, float ArrowSize = 10.0f, const FColor& Color = FColor::Yellow, float Thickness = 2.0f, float LifeTime = -1.0f);
		static void Sphere(const FVector& Center, float Radius, const FColor& Color = FColor::Cyan, float LifeTime = -1.0f);
		static void Box(const FVector& Center, const FVector& Extents, const FQuat& Rotation, const FColor& Color = FColor::Green, float LifeTime = -1.0f);
		static void CoordinateSystem(const FVector& Location, const FRotator& Rotation, float Scale = 50.0f, float LifeTime = -1.0f);
	};

	// 현실 세계 값
	struct FZCRealWorldConsts
	{
	public:
		// 물의 밀도 kg / m3
		static constexpr float WaterDensity = 997.0f;
		// 공기 밀도 kg / m3
		static constexpr float AirDensity = 1.225f;
		// 공기 밀도 절반 kg / m3
		static constexpr float HalfAirDensity = 0.6125f;
		// 건조한 도로 마찰 계수
		static constexpr float DryRoadFriction = 0.7f;
		// 젖은 도로 마찰 계수
		static constexpr float WetRoadFriction = 0.4f;
	};

	class ZCGearUtility
	{
	public:
		// 입력값을 0~1사이로 정규화
		FORCEINLINE static float ClampNormalRange(float& InValue)
		{
			return FMath::Clamp(InValue, 0.0f, 1.0f);
		}

		// 기어의 앞방향 기준으로 좌우 회전 각도(Yaw)를 라디안으로 반환
		static float YawFromForwardVectorRadians(const FVector& NormalizedForwardsVector)
		{
			return FMath::Atan2(NormalizedForwardsVector.Y, NormalizedForwardsVector.X);
		}

		// 기어의 앞방향을 기준으로 위아래 기울기(Pitch)를 라디안으로 반환
		static float PitchFromForwardVectorRadians(const FVector& NormalizedForwardsVector)
		{
			return FMath::Atan2(NormalizedForwardsVector.Z, FMath::Sqrt(NormalizedForwardsVector.X * NormalizedForwardsVector.X + NormalizedForwardsVector.Y * NormalizedForwardsVector.Y));
		}

		// 기어의 오른쪽 방향 벡터를 기준으로 좌우 기울어짐(Roll)을 라디안으로 반환
		static float RollFromRightVectorRadians(const FVector& NormalizedRightVector)
		{
			return FMath::Atan2(NormalizedRightVector.Z, FMath::Sqrt(NormalizedRightVector.X * NormalizedRightVector.X + NormalizedRightVector.Y * NormalizedRightVector.Y));
		}

		// 슬립 각도 계산, 조종이 가능한 기어의 이동 방향과 실제로 나아가는 방향 사이의 각도를 계산
		static float CalculateSlipAngle(float Y, float X)
		{
			float Value = 0.0f;

			float LateralSpeedThreshold = 0.05f;
			if (FMath::Abs(Y) > LateralSpeedThreshold)
			{
				Value = FMath::Abs(FMath::Atan2(Y, X));
				if (Value > HALF_PI)
				{
					Value = PI - Value;
				}
			}

			return Value;
		}
	};

	/** 미터(m)를 센티미터(cm)로 변환하는 배율 */
	FORCEINLINE float MToCmScaling()
	{
		return 100.f;
	}

	/** 센티미터(cm)를 미터(m)로 변환하는 배율 */
	FORCEINLINE float CmToMScaling()
	{
		return 0.01f;
	}

	/** 분당 회전수(RPM)를 각속도(radians per second)로 변환 */
	FORCEINLINE float RPMToOmega(float RPM)
	{
		return RPM * PI / 30.f;
	}

	/** 각속도(radians per second)를 분당 회전수(RPM)로 변환 */
	FORCEINLINE float OmegaToRPM(float Omega)
	{
		return Omega * 30.f / PI;
	}

	/** 시속(km/h)을 초속 센티미터(cm/s)로 변환 */
	FORCEINLINE float KmHToCmS(float KmH)
	{
		return KmH * 100000.f / 3600.f;
	}

	/** 초속 센티미터(cm/s)를 시속(km/h)으로 변환 */
	FORCEINLINE float CmSToKmH(float CmS)
	{
		return CmS * 3600.f / 100000.f;
	}

	/** 초속 센티미터(cm/s)를 시속 마일(mph)로 변환 */
	FORCEINLINE float CmSToMPH(float CmS)
	{
		return CmS * 2236.94185f / 100000.f;
	}

	/** 시속 마일(mph)을 초속 센티미터(cm/s)로 변환 */
	FORCEINLINE float MPHToCmS(float MPH)
	{
		return MPH * 100000.f / 2236.94185f;
	}

	/** 시속 마일(mph)을 초속 미터(m/s)로 변환 */
	FORCEINLINE float MPHToMS(float MPH)
	{
		return MPH * 1609.34f / 3600.f;
	}

	/** 초속 미터(m/s)를 시속 마일(mph)로 변환 */
	FORCEINLINE float MSToMPH(float MS)
	{
		return MS * 3600.f / 1609.34f;
	}

	/** 센티미터(cm)를 미터(m)로 변환 (단일 값) */
	FORCEINLINE float CmToM(float Cm)
	{
		return Cm * 0.01f;
	}

	/** 센티미터(cm) 벡터를 미터(m) 벡터로 변환 */
	FORCEINLINE FVector CmToM(const FVector& Cm)
	{
		return Cm * 0.01f;
	}

	/** 미터(m)를 센티미터(cm)로 변환 (단일 값) */
	FORCEINLINE float MToCm(float M)
	{
		return M * 100.0f;
	}

	/** 미터(m) 벡터를 센티미터(cm) 벡터로 변환 */
	FORCEINLINE FVector MToCm(const FVector& M)
	{
		return M * 100.0f;
	}

	/** 센티미터(cm)를 마일(Miles)로 변환 - 정밀도 손실 주의 */
	FORCEINLINE float CmToMiles(float Cm)
	{
		return Cm * 0.0000062137119224f;
	}

	/** 킬로미터(km)를 마일(Miles)로 변환 */
	FORCEINLINE float KmToMile(float Km)
	{
		return Km * 0.62137f;
	}

	/** 마일(Miles)을 킬로미터(km)로 변환 */
	FORCEINLINE float MileToKm(float Miles)
	{
		return Miles * 1.60934f;
	}

	/** 제곱미터(m²)를 제곱센티미터(cm²)로 변환 */
	FORCEINLINE float M2ToCm2(float M2)
	{
		return M2 * 100.f * 100.f;
	}

	/** 제곱센티미터(cm²)를 제곱미터(m²)로 변환 */
	FORCEINLINE float Cm2ToM2(float Cm2)
	{
		return Cm2 / (100.f * 100.f);
	}

	/** 도(Degree)를 라디안(Radian)으로 변환 */
	FORCEINLINE float DegToRad(float InDeg)
	{
		return InDeg * PI / 180.f;
	}

	/** 라디안(Radian)을 도(Degree)로 변환 */
	FORCEINLINE float RadToDeg(float InRad)
	{
		return InRad * 180.f / PI;
	}

	/** 입력값의 제곱을 계산 */
	FORCEINLINE float Sqr(float Val)
	{
		return Val * Val;
	}

	/** 미터 단위 토크 값을 센티미터 단위 토크로 변환 (10,000 배율) */
	FORCEINLINE float TorqueMToCm(float TorqueIn)
	{
		return TorqueIn * 10000.0f;
	}

	/** 센티미터 단위 토크 값을 미터 단위 토크로 변환 (10,000으로 나눔) */
	FORCEINLINE float TorqueCmToM(float TorqueIn)
	{
		return TorqueIn / 10000.0f;
	}

}

#undef ZC_API
