// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Config/MassUnitSystemSettings.h"

UMassUnitSystemSettings::UMassUnitSystemSettings()
{
	CategoryName = TEXT("Plugins");
	LODDistanceThresholds = {500.0f, 1500.0f, 3000.0f, 6000.0f};
}

UMassUnitSystemSettings* UMassUnitSystemSettings::Get()
{
	return GetMutableDefault<UMassUnitSystemSettings>();
}
