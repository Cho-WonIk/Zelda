
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class IZeldaPhysics : public IModuleInterface
{
public:
	static inline IZeldaPhysics& Get()
	{
		return FModuleManager::LoadModuleChecked<IZeldaPhysics>("ZeldaPhysics");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ZeldaPhysics");
	}

private:

	void PhysSceneInit(FPhysScene* PhysScene);
	void PhysSceneTerm(FPhysScene* PhysScene);

	FDelegateHandle OnPhysSceneInitHandle;
	FDelegateHandle OnPhysSceneTermHandle;
};
