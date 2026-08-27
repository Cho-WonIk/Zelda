
#pragma once

#include "CoreMinimal.h"
#include "Runtime/ZCGearType.h"
//#include "ZCSimGearInput.generated.h"

/*
바퀴류
X축 : 바퀴의 회전 방향 및 회전 속도
Y축 : 바퀴의 좌우 기울기, 0~1 사이로 0도에서 45도까지

조종간에서의 로직
연결된 액터들의 질량 중심을 구하고 해당 지점에 토크를 줌

X축 : Pitch 토크
Y축 : Roll과 Yaw 토크에 적절하게 분배

*/

namespace ZCGear
{

	// ==================================== 기어 입력 버퍼 컨테이너 ===================================

	// 기어 시뮬레이션에 적용할 값, 적용 방식(더하기, 곱하기, 덮어쓰기), 값
	struct FZCSimGearValue
	{
		FZCSimGearValue() {};
		FZCSimGearValue(EZCGearInputApplyType InApplyType, float InValue)
			: ApplyType(InApplyType)
			, Value(InValue)
		{}

		EZCGearInputApplyType ApplyType = EZCGearInputApplyType::None;
		float Value = 0.0f;
	};

	// 기어 축 세트, 각 축별 FSimGearValue 저장
	struct FZCSimGearModuleSet
	{
		TArray<FZCSimGearValue> AxisValues;

		FZCSimGearModuleSet();

		void Reset();

		FZCSimGearValue& operator[](EZCGearAxisType AxisType)
		{
			return AxisValues[(int32)AxisType];
		}

		const FZCSimGearValue& operator[](EZCGearAxisType AxisType) const
		{
			return AxisValues[(int32)AxisType];
		}
	};

	// 플레이어의 입력을 각 기어에서 사용할 수 있는 형태로 저장(게임 스레드용)
	class FZCSimGearInputBufferContainer
	{

	public:
		FZCSimGearInputBufferContainer();

		const FZCSimGearModuleSet& GetModuleInputSet(EZCGearType GearType) const
		{
			return ModuleInputs[(int32)GearType];
		}

		FZCSimGearModuleSet& AccessModuleInputSet(EZCGearType GearType)
		{
			return ModuleInputs[(int32)GearType];
		}

		const FZCSimGearValue& GetModuleInputYSyncValues(EZCGearType GearType) const
		{
			return ModuleInputYSyncValues[(int32)GearType];
		}

		FZCSimGearValue& AccessModuleInputYSyncValues(EZCGearType GearType)
		{
			return ModuleInputYSyncValues[(int32)GearType];
		}

		void SetCenterOfMassOffset(const FVector& InOffset) { CenterOfMassOffset = InOffset; }
		const FVector& GetCenterOfMassOffset() const { return CenterOfMassOffset; }

		void SetTotalMass(const float InMass) { TotalMass = InMass; }
		float GetTotalMass() const { return TotalMass; }

		void SetPlayerViewDirection(const FVector& InDirection) { PlayerViewDirection = InDirection; }
		const FVector& GetPlayerViewDirection() const { return PlayerViewDirection; }

		void ClearBuffers();

	protected:
		// 기어 모듈별 입력 세트
		TArray< FZCSimGearModuleSet > ModuleInputs;

		// 기어 모듈별, 입력 Y축 동기화 여부 및 값
		TArray< FZCSimGearValue > ModuleInputYSyncValues;

		// 인풋 컴포넌트에서 연결된 전체 액터들의 무게 중심 오프셋
		FVector CenterOfMassOffset = FVector::ZeroVector;

		// 플레이어 캐릭터가 현재 바라보는 방향 (월드 벡터 기준)
		FVector PlayerViewDirection = FVector::ZeroVector;

		float TotalMass = 0.0f;
	};


	/*=========================== 입력 버퍼에서 기어 시뮬레이션으로 보낼 값 ==========================*/

	struct FZCSimGearModuleInputBase
	{
		FZCSimGearModuleInputBase() = default;
		virtual ~FZCSimGearModuleInputBase() = default;

		EZCGearType GearType = EZCGearType::Undefined;

		virtual void Reset() = 0;
	};

	/**
	* 바퀴 모듈 입력 데이터
	* Pitch : 바퀴 회전 방향
	*/
	struct FZCSimGearWheelModuleInput : public FZCSimGearModuleInputBase
	{
		FZCSimGearWheelModuleInput()
		{
			GearType = EZCGearType::Wheel;

		}

		// 피치 입력
		float Pitch = 0.0f;

		// TODO : 요 입력
		float Yaw = 0.0f;

		virtual void Reset() override
		{
			Pitch = 0.0f;
			Yaw = 0.0f;
		}
	};

	/**
	* 입력 모듈 입력 데이터
	* X축(앞뒤) 및 yaw, pitch 입력
	*/
	struct FZCSimGearInputModuleInput : public FZCSimGearModuleInputBase
	{
		FZCSimGearInputModuleInput()
		{
			GearType = EZCGearType::Input;
		}

		float YAxis = 0.0f;
		float Yaw = 0.0f;
		float Pitch = 0.0f;

		virtual void Reset() override
		{
			YAxis = 0.0f;
			Yaw = 0.0f;
			Pitch = 0.0f;
		}

	};

	// 물리 스레드로 전달할 모듈 입력 컨테이너, 같은 모듈이면 같은 값을 받음
	struct FZCSimGearModuleInputContainer
	{
		FZCSimGearModuleInputContainer() = default;

		FZCSimGearWheelModuleInput WheelInput;
		FZCSimGearInputModuleInput InputInput;

		// 플레이어 입력 방향과 동기화 시 사용할 벡터, 월드 벡터 기준
		FVector CockpitDirection = FVector::ZeroVector;

		// 입력을 받게 되는 액터의 무게 중심 벡터(조종간 기준 상대 오프셋, InputModule에서 만 사용)
		FVector ActorCenterOfMassOffset = FVector::ZeroVector;

		// 조종하게 되는 액터들의 총무게
		float TotalMass = 1.0f;

		// 유효한 입력이 있는지 여부
		uint8 bHasInput : 1 = false;

		void Reset()
		{
			WheelInput.Reset();
			CockpitDirection = FVector::ZeroVector;
			ActorCenterOfMassOffset = FVector::ZeroVector;
			bHasInput = false;
		}

		// 입력 버퍼 컨테이너에서 모듈별 입력 추출
		void ExtractFromInputBuffer(const FZCSimGearInputBufferContainer& InputBuffer);

		// 특정 모듈 입력에 접근
		const FZCSimGearModuleInputBase* GetModuleInput(EZCGearType GearType) const;
	};

}

