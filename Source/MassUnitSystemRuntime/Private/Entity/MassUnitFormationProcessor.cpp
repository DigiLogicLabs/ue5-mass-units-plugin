// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitFormationProcessor.h"
#include "MassUnitCommonFragments.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityView.h"

UMassUnitFormationProcessor::UMassUnitFormationProcessor()
{
    // ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement); // Deprecated
    // ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::LOD); // Deprecated
}

void UMassUnitFormationProcessor::SetupUnitQueries()
{
    EntityQuery.AddRequirement<FMassUnitTransformFragment>((int)EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassUnitFormationFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitTargetFragment>((int)EMassFragmentAccess::ReadWrite);
}

void UMassUnitFormationProcessor::ExecuteFallback(FMassUnitEntityManagerFallback& EntityManager, FMassUnitExecutionContext& Context)
{
    // ...existing fallback logic...
}

void UMassUnitFormationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Unreal override for compatibility. Redirect to fallback if needed.
}
