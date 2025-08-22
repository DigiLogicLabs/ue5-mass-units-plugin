// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Core/MassUnitSystemRuntime.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FMassUnitSystemRuntimeModule, MassUnitSystemRuntime);

void FMassUnitSystemRuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("MassUnitSystemRuntime: Module started"));
}

void FMassUnitSystemRuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("MassUnitSystemRuntime: Module shutdown"));
}
