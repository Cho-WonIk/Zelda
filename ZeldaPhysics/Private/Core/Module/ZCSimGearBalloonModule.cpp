
#include "Core/Module/ZCSimGearBalloonModule.h"

namespace ZCGear
{
	FZCSimGearBalloonModule::FZCSimGearBalloonModule(const FZCBalloonSettings& Settings)
		: TZCSimGearModuleSettings<FZCBalloonSettings>(Settings)
	{
	}

	void FZCSimGearBalloonModule::Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs)
	{
		const FZCBalloonSettings& CurrentSettings = Setup();

		float AppliedForceMag = CurrentSettings.MaxForce;

		//UE_LOG(LogTemp, Warning, TEXT("Balloon Module Simulate Called. MaxForce: %f, MaxSpeed: %f"), CurrentSettings.MaxForce, CurrentSettings.MaxSpeed);

		if (Inputs.HasValidInputContainer())
		{
			float TotalMass = Inputs.ModuleInputContainer->TotalMass;
			AppliedForceMag *= TotalMass;

			//UE_LOG(LogTemp, Warning, TEXT("Balloon Module TotalMass: %f, Adjusted Force: %f"), TotalMass, AppliedForceMag);
		}

		const FVector WorldUp = FVector::UpVector * AppliedForceMag;// * DeltaTime;

		AddSpeedLimitedForce(WorldUp, CurrentSettings.MaxSpeed, true, false, false, false, FColor::Red);


	}
}