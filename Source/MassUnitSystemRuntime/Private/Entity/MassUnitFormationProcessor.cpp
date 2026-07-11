// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitFormationProcessor.h"

#include "Entity/MassUnitFragments.h"

UMassUnitFormationProcessor::UMassUnitFormationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false;
}

void UMassUnitFormationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassUnitFormationFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassUnitFormationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// UFormationSystem performs this work because it owns formation handles and slot layouts.
}
