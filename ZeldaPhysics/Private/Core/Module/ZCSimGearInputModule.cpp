
#include "Core/Module/ZCSimGearInputModule.h"

#include "Core/ZCGearUtility.h"

#include "Core/ZCSimGearSimulation.h"

namespace ZCGear
{
	FZCSimGearInputModule::FZCSimGearInputModule(const FZCInputSettings& Settings)
		: TZCSimGearModuleSettings<FZCInputSettings>(Settings)
	{
	}

	void FZCSimGearInputModule::Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs)
	{
		if (Inputs.HasValidInputContainer())
		{
			const FZCSimGearModuleInputContainer& Container = *Inputs.ModuleInputContainer;
			const FZCSimGearInputModuleInput& Input = Container.InputInput;

			float YawValue = Input.Yaw;

			if (!FMath::IsNearlyZero(YawValue))
			{
				float TorqueMagnitude = YawValue * 1000.0f * Container.TotalMass * 100.0f;

				FVector CalculatedTorque = FVector::UpVector * TorqueMagnitude;

				// SetTorque 대신 AddLocalTorque 호출
				AddLocalTorque(
					CalculatedTorque,
					true,   // bAllowSubstepping
					true,   // bAccelChange (가속도 변화로 적용하여 질량 무시 가능)
					FColor::Yellow
				);
			}
		}
	}
}