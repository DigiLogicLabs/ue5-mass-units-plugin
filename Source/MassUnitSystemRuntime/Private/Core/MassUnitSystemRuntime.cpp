// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Core/MassUnitSystemRuntime.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogMassUnitSystem);

IMPLEMENT_MODULE(FMassUnitSystemRuntimeModule, MassUnitSystemRuntime);

void FMassUnitSystemRuntimeModule::StartupModule()
{
	UE_LOG(LogMassUnitSystem, Log, TEXT("Mass Unit System runtime module started"));
}

void FMassUnitSystemRuntimeModule::ShutdownModule()
{
	UE_LOG(LogMassUnitSystem, Log, TEXT("Mass Unit System runtime module shut down"));
}
