
#include "Core/Module/ZCSimGearFanModule.h"

namespace ZCGear
{
	FZCSimGearFanModule::FZCSimGearFanModule(const FZCFanSettings& Settings)
		: TZCSimGearModuleSettings<FZCFanSettings>(Settings)
	{
		//DisabledPhysicsType = EZCSimGearPhysiscType::CustomPhysics;
	}

	void FZCSimGearFanModule::Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs)
	{

        float AppliedForceMag = Setup().DefaultForce;
        if (Inputs.HasValidInputContainer())
        {
            float TotalMass = Inputs.ModuleInputContainer->TotalMass;
            AppliedForceMag *= TotalMass;
        }

        const FVector LocalForce = FVector(0.0f, 1.0f, 0.0f) * AppliedForceMag * DeltaTime;

        AddSpeedLimitedForce(LocalForce, Setup().MaxSpeed, true, false, true, false, FColor::Orange);
	}
}
