// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Zelda : ModuleRules
{
	public Zelda(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "Zelda"});

        PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara", 
			"HairStrandsCore", "AnimGraphRuntime", "MotionWarping", "UMG", "Slate", "SlateCore", 
			"GameplayTags", "GameplayStateTreeModule", "PropertyBindingUtils", "CommonInput", 
			"CommonUI", "AIModule", "NavigationSystem", "DeveloperSettings", "PhysicsControl",
			"GameplayTasks", "Chooser", "PhysicsCore", "RenderCore", "RHI" });
	}
}
