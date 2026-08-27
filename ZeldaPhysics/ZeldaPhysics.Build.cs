
using UnrealBuildTool;

public class ZeldaPhysics : ModuleRules
{
	public ZeldaPhysics(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "ZeldaPhysics" });

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				"InputCore",
                "DeveloperSettings",
                "AnimGraphRuntime",
                "Chaos",
                "PhysicsCore",
                "GeometryCollectionEngine"
            }
		);
	}
}