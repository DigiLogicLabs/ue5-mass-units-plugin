// Copyright Your Company. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MassUnitSystemRuntime : ModuleRules
{
	public MassUnitSystemRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Always define WITH_GASCOMPANION=0 for now
		// This can be changed to 1 if GASCompanion is available in the project
		PublicDefinitions.Add("WITH_GASCOMPANION=0");
		
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
				"MassEntity",
				"Niagara",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"NavigationSystem",
				"AIModule"
				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"RenderCore",
				"RHI",
				"Projects",
				"NetCore",
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
