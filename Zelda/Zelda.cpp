// Copyright Epic Games, Inc. All Rights Reserved.

#include "Zelda.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"

void FZelda::StartupModule()
{
	FString BaseDir = FPaths::ProjectDir();

	FString ShaderDir = FPaths::Combine(BaseDir, TEXT("Shaders"));

	AddShaderSourceDirectoryMapping(TEXT("/Zelda/Shader"), ShaderDir);
}

void FZelda::ShutdownModule()
{
}

IMPLEMENT_PRIMARY_GAME_MODULE(FZelda, Zelda, "Zelda");
