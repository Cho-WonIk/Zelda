
using UnrealBuildTool;

public class ZeldaEditor : ModuleRules
{
	public ZeldaEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "ZeldaEditor" });
	
		PublicDependencyModuleNames.AddRange(
			new string[] 
			{ 
				"Core", 
				"UMG",
				"Blutility",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"UnrealEd",
				"AssetTools",
				"PropertyEditor",
				"AdvancedPreviewScene",
				"EditorStyle",
				"EditorWidgets",
				"Projects",
				"AdvancedPreviewScene",
				"Persona",
				"DeveloperSettings",
				"Zelda"
		});
	}
}
