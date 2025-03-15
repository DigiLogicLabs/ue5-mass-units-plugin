// Copyright Your Company. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MassUnitSystemTarget : TargetRules
{
	public MassUnitSystemTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange(new string[] { "MassUnitSystemRuntime", "MassUnitSystemEditor" });
	}
}
