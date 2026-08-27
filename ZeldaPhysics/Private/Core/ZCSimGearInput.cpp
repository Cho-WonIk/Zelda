
#include "Core/ZCSimGearInput.h"

namespace ZCGear
{
	FZCSimGearModuleSet::FZCSimGearModuleSet()
	{
		const int32 AxisCount = (int32)EZCGearAxisType::Max;
		AxisValues.SetNum(AxisCount);
	}

	void FZCSimGearModuleSet::Reset()
	{
		for (auto& Value : AxisValues)
		{
			Value = FZCSimGearValue();
		}
	}


	FZCSimGearInputBufferContainer::FZCSimGearInputBufferContainer()
	{
		ModuleInputs.SetNum((int32)EZCGearType::Max);

		ModuleInputYSyncValues.SetNum((int32)EZCGearType::Max);
	}

	void FZCSimGearInputBufferContainer::ClearBuffers()
	{
		for (auto& ModuleInput : ModuleInputs)
		{
			ModuleInput.Reset();
		}
		for (auto& YSyncValue : ModuleInputYSyncValues)
		{
			YSyncValue = FZCSimGearValue();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////


	void FZCSimGearModuleInputContainer::ExtractFromInputBuffer(const FZCSimGearInputBufferContainer& InputBuffer)
	{
		Reset();

		bool bHasAnyInput = true;

		// 무게 중심 오프셋 추출
		ActorCenterOfMassOffset = InputBuffer.GetCenterOfMassOffset();

		// 플레이어 뷰 방향 추출
		CockpitDirection = InputBuffer.GetPlayerViewDirection();

		TotalMass = InputBuffer.GetTotalMass();

		// 바퀴 모듈 입력 추출
		{
			const FZCSimGearModuleSet& WheelModuleSet = InputBuffer.GetModuleInputSet(EZCGearType::Wheel);
			const FZCSimGearValue& YSyncValue = InputBuffer.GetModuleInputYSyncValues(EZCGearType::Wheel);

			const FZCSimGearValue& PitchValue = WheelModuleSet[EZCGearAxisType::Pitch];

			WheelInput.Pitch = PitchValue.Value;
			WheelInput.Yaw = YSyncValue.Value;

		}

		// 입력 모듈 입력 추출
		{
			const FZCSimGearModuleSet& InputModuleSet = InputBuffer.GetModuleInputSet(EZCGearType::Input);

			const FZCSimGearValue& YAxisValue = InputModuleSet[EZCGearAxisType::YAxis];
			const FZCSimGearValue& YawValue = InputModuleSet[EZCGearAxisType::Yaw];
			const FZCSimGearValue& PitchValue = InputModuleSet[EZCGearAxisType::Pitch];

			InputInput.YAxis = YAxisValue.Value;
			InputInput.Yaw = YawValue.Value;
			InputInput.Pitch = PitchValue.Value;

		}

		bHasInput = bHasAnyInput;
	}

	const FZCSimGearModuleInputBase* FZCSimGearModuleInputContainer::GetModuleInput(EZCGearType GearType) const
	{
		switch (GearType)
		{
		case EZCGearType::Wheel:
			return &WheelInput;
		case EZCGearType::Input:
			return &InputInput;
		default:
			return nullptr;
		}
	}
}
