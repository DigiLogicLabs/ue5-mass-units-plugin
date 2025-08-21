// Copyright Your Company. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class MassUnitSystemEditor : ModuleRules
{
	public MassUnitSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Define WITH_GASCOMPANION=1 since we know it's available in this project
		PublicDefinitions.Add("WITH_GASCOMPANION=1");
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);
				
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add private include paths required here ...
			}
		);
			
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"MassUnitSystemRuntime",
				"Niagara",
				"GameplayAbilities",
				"GameplayTags"
				// ... add other public dependencies that you statically link with here ...
			}
		);
		
		// Conditionally add MassEntity if available
		try
		{
			// MassEntity is now part of the engine in UE5.6+
		}
		catch (Exception)
		{
			// MassEntity is not available, use the fallback
			PublicDefinitions.Add("WITH_MASSENTITY=0"); // No longer needed, kept for legacy compatibility
		}
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"Slate",
				"SlateCore",
				"PropertyEditor",
				"EditorStyle",
				"EditorWidgets",
				"AssetTools",
				"AssetRegistry",
				"DetailCustomizations",
				"Projects",
				"DeveloperSettings"
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}
