// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Module for the Mass Unit System runtime functionality
 */
class FMassUnitSystemRuntimeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FMassUnitSystemRuntimeModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FMassUnitSystemRuntimeModule>("MassUnitSystemRuntime");
	}

	/**
	 * Checks to see if this module is loaded and ready.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("MassUnitSystemRuntime");
	}
};
