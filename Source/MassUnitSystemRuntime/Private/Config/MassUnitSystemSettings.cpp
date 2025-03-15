// Copyright Your Company. All Rights Reserved.

#include "Config/MassUnitSystemSettings.h"
#include "Config/UnitConfigDataAsset.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

UMassUnitSystemSettings::UMassUnitSystemSettings()
    : MaxUnits(10000)
    , MaxSkeletalMeshUnits(100)
    , MaxPathRequestsPerFrame(100)
    , bEnableDebugVisualization(false)
    , DebugVisualizationDuration(5.0f)
{
}

UMassUnitSystemSettings* UMassUnitSystemSettings::Get()
{
    // Get the default object
    UMassUnitSystemSettings* Settings = GetMutableDefault<UMassUnitSystemSettings>();
    
    // Return the settings
    return Settings;
}
