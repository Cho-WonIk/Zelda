
#include "ZeldaPhysics.h"
#include "Modules/ModuleManager.h"
#include "ZCSimGearManager.h"

class FZeldaPhysics : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		check(GConfig);
		OnPhysSceneInitHandle = FPhysicsDelegates::OnPhysSceneInit.AddRaw(this, &FZeldaPhysics::PhysSceneInit);
		OnPhysSceneTermHandle = FPhysicsDelegates::OnPhysSceneTerm.AddRaw(this, &FZeldaPhysics::PhysSceneTerm);
	}

	virtual void ShutdownModule() override
	{
		FPhysicsDelegates::OnPhysSceneInit.Remove(OnPhysSceneInitHandle);
		FPhysicsDelegates::OnPhysSceneTerm.Remove(OnPhysSceneTermHandle);
	}

	void PhysSceneInit(FPhysScene* PhysScene)
	{
		new FZCSimGearManager(PhysScene);
	}

	void PhysSceneTerm(FPhysScene* PhysScene)
	{
		FZCSimGearManager* GearManger = FZCSimGearManager::GetManagerFromScene(PhysScene);
		if (!GearManger) return;

		delete GearManger;
		GearManger = nullptr;
	}

	FDelegateHandle OnPhysSceneInitHandle;
	FDelegateHandle OnPhysSceneTermHandle;
};

IMPLEMENT_MODULE(FZeldaPhysics, ZeldaPhysics)