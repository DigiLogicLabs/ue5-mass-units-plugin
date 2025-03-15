// Copyright Your Company. All Rights Reserved.

#include "Entity/MassUnitFormationProcessor.h"
#include "MassCommonFragments.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityView.h"

UMassUnitFormationProcessor::UMassUnitFormationProcessor()
{
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::LOD);
}

void UMassUnitFormationProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassUnitFormationFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitTargetFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassUnitFormationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Process entities
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassUnitFormationFragment> FormationList = Context.GetMutableFragmentView<FMassUnitFormationFragment>();
        const TArrayView<FMassUnitTargetFragment> TargetList = Context.GetMutableFragmentView<FMassUnitTargetFragment>();
        
        // Group entities by formation
        TMap<int32, TArray<int32>> FormationMap;
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FMassUnitFormationFragment& FormationFragment = FormationList[EntityIndex];
            
            // Skip entities not in a formation
            if (!FormationFragment.IsInFormation())
            {
                continue;
            }
            
            // Add to formation map
            FormationMap.FindOrAdd(FormationFragment.FormationHandle).Add(EntityIndex);
        }
        
        // Process each formation
        for (const auto& FormationPair : FormationMap)
        {
            int32 FormationHandle = FormationPair.Key;
            const TArray<int32>& EntityIndices = FormationPair.Value;
            
            // Skip empty formations
            if (EntityIndices.Num() == 0)
            {
                continue;
            }
            
            // Calculate formation center
            FVector FormationCenter = FVector::ZeroVector;
            for (int32 EntityIndex : EntityIndices)
            {
                FormationCenter += TransformList[EntityIndex].GetTransform().GetLocation();
            }
            FormationCenter /= EntityIndices.Num();
            
            // Calculate formation positions for each entity
            for (int32 EntityIndex : EntityIndices)
            {
                FMassUnitFormationFragment& FormationFragment = FormationList[EntityIndex];
                FMassUnitTargetFragment& TargetFragment = TargetList[EntityIndex];
                
                // Calculate formation position
                FVector FormationPosition = CalculateFormationPosition(EntityManager, Context.GetEntity(EntityIndex), FormationHandle);
                
                // Update formation offset
                FVector CurrentOffset = FormationFragment.FormationOffset;
                FVector TargetOffset = FormationPosition - FormationCenter;
                
                // Blend towards target offset
                FormationFragment.FormationOffset = FMath::Lerp(CurrentOffset, TargetOffset, AdaptationRate);
                
                // Set target location to formation position
                TargetFragment.TargetLocation = FormationCenter + FormationFragment.FormationOffset;
            }
        }
    });
}

FVector UMassUnitFormationProcessor::CalculateFormationPosition(FMassEntityManager& EntityManager, FMassEntityHandle Entity, int32 FormationHandle)
{
    // Get entity view
    FMassEntityView EntityView(EntityManager, Entity);
    
    // Get fragments
    const FMassUnitFormationFragment& FormationFragment = EntityView.GetFragmentData<FMassUnitFormationFragment>();
    const FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
    
    // Get formation slot
    int32 FormationSlot = FormationFragment.FormationSlot;
    
    // In a real implementation, we would have different formation types and calculate positions accordingly
    // For this example, we'll implement a simple line formation
    
    // Calculate row and column based on slot
    int32 UnitsPerRow = 10; // Arbitrary number for this example
    int32 Row = FormationSlot / UnitsPerRow;
    int32 Column = FormationSlot % UnitsPerRow;
    
    // Calculate offset based on row and column
    FVector Offset;
    Offset.X = (Column - UnitsPerRow / 2) * UnitSpacing;
    Offset.Y = Row * UnitSpacing;
    Offset.Z = 0.0f;
    
    return Offset;
}
