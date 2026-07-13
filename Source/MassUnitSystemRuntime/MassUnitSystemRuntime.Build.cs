// Copyright Digi Logic Labs LLC. All Rights Reserved.

using UnrealBuildTool;

public class MassUnitSystemRuntime : ModuleRules
{
	public MassUnitSystemRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"MassEntity",
				"MassCommon",
				"MassSimulation",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"NavigationSystem",
				"AIModule",
				"Niagara",
				"DeveloperSettings"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"RenderCore"
			}
		);
	}
}
