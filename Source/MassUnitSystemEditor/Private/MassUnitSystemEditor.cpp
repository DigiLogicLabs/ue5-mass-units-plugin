// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "MassUnitSystemEditor.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMassUnitSystemEditor, Log, All);

IMPLEMENT_MODULE(FMassUnitSystemEditorModule, MassUnitSystemEditor);

void FMassUnitSystemEditorModule::StartupModule()
{
	UE_LOG(LogMassUnitSystemEditor, Verbose, TEXT("Mass Unit System editor module started"));
}

void FMassUnitSystemEditorModule::ShutdownModule()
{
	UE_LOG(LogMassUnitSystemEditor, Verbose, TEXT("Mass Unit System editor module shut down"));
}
