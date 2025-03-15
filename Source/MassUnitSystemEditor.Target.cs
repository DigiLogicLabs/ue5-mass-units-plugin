// Copyright Your Company. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MassUnitSystemEditorTarget : TargetRules
{
	public MassUnitSystemEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange(new string[] { "MassUnitSystemRuntime", "MassUnitSystemEditor" });
	}
}
