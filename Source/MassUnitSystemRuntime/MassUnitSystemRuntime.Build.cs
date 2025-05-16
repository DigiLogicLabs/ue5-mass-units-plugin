// Copyright Your Company. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class MassUnitSystemRuntime : ModuleRules
{
	public MassUnitSystemRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				"Niagara",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"NavigationSystem",
				"AIModule",
				"MassEntity",
				"MassCommon",
				"MassActors",
				"MassGameplay",
				"MassSignals",
				"MassSpawner",
				"MassRepresentation",
				"MassNavigation",
				"MassAIBehavior",
				"MassZoneGraphNavigation",
				"MassGameplayExternalTraits",
				"MassMovement",
				"MassSmartObjects",
				"MassReplication"
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
