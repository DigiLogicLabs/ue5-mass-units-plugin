// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "MassUnitSystemEditor.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FMassUnitSystemEditorModule, MassUnitSystemEditor);

void FMassUnitSystemEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("MassUnitSystemEditor: Module started"));
}

void FMassUnitSystemEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("MassUnitSystemEditor: Module shutdown"));
}
