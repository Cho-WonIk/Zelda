
#pragma once

#include "Development/ZCDebug.h"
#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING

bool Zelda::Debug::Ultrahand::bDrawDebugAll = false;
bool Zelda::Debug::Ultrahand::bDrawDevice = false;
bool Zelda::Debug::Ultrahand::bDrawGear = false;
bool Zelda::Debug::Ultrahand::bDrawAssemble = false;

static FAutoConsoleVariableRef CVar_DebugAll(
	Zelda::Debug::Ultrahand::all,
	Zelda::Debug::Ultrahand::bDrawDebugAll,
	TEXT("Ultrahand 디버깅 전체 On/Off"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVar_DebugDevice(
	Zelda::Debug::Ultrahand::Device,
	Zelda::Debug::Ultrahand::bDrawDevice,
	TEXT("Ultrahand - 디바이스 디버깅 시각화"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVar_DebugGear(
	Zelda::Debug::Ultrahand::Gear,
	Zelda::Debug::Ultrahand::bDrawGear,
	TEXT("Ultrahand - 조나우기어 디버깅"),
	ECVF_Default
);

static FAutoConsoleVariableRef CVar_DebugAssemble(
	Zelda::Debug::Ultrahand::Assemble,
	Zelda::Debug::Ultrahand::bDrawAssemble,
	TEXT("Ultrahand - 어셈블액터 디버깅"),
	ECVF_Default
);

#endif