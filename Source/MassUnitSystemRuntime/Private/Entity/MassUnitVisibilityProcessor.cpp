// Copyright Your Company. All Rights Reserved.

#include "Entity/MassUnitVisibilityProcessor.h"
#include "MassCommonFragments.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityView.h"
#include "MassLODFragments.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UMassUnitVisibilityProcessor::UMassUnitVisibilityProcessor()
{
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
}

void UMassUnitVisibilityProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassUnitVisualFragment>(EMassFragmentAccess::ReadWrite);
    
    // Add LOD fragment requirements
    EntityQuery.AddRequirement<FMassLODFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVisualizationLODFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassUnitVisibilityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player view location
    FVector ViewLocation = FVector::ZeroVector;
    bool bHasViewLocation = false;
    
    // Get first local player controller
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        if (PlayerController && PlayerController->IsLocalController())
        {
            FVector ViewLocationTemp;
            FRotator ViewRotation;
            PlayerController->GetPlayerViewPoint(ViewLocationTemp, ViewRotation);
            
            ViewLocation = ViewLocationTemp;
            bHasViewLocation = true;
            break;
        }
    }
    
    // Skip if no view location
    if (!bHasViewLocation)
    {
        return;
    }
    
    // Process entities
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager, ViewLocation](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassUnitVisualFragment> VisualList = Context.GetMutableFragmentView<FMassUnitVisualFragment>();
        const TArrayView<FMassLODFragment> LODList = Context.GetMutableFragmentView<FMassLODFragment>();
        const TArrayView<FMassVisualizationLODFragment> VisLODList = Context.GetMutableFragmentView<FMassVisualizationLODFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& TransformFragment = TransformList[EntityIndex];
            FMassUnitVisualFragment& VisualFragment = VisualList[EntityIndex];
            FMassLODFragment& LODFragment = LODList[EntityIndex];
            FMassVisualizationLODFragment& VisLODFragment = VisLODList[EntityIndex];
            
            // Determine visibility state
            int32 NewLODLevel = DetermineVisibilityState(EntityManager, Context.GetEntity(EntityIndex), ViewLocation);
            
            // Update LOD level
            VisualFragment.LODLevel = NewLODLevel;
            LODFragment.Level = NewLODLevel;
            VisLODFragment.LODLevel = NewLODLevel;
            
            // Determine if entity should be visible
            VisualFragment.bIsVisible = (NewLODLevel < LODDistanceThresholds.Num());
            
            // Determine if entity should use skeletal mesh
            float DistanceSquared = FVector::DistSquared(TransformFragment.GetTransform().GetLocation(), ViewLocation);
            bool bShouldUseSkeletalMesh = (DistanceSquared <= FMath::Square(SkeletalMeshDistance));
            
            // Update skeletal mesh flag
            if (bShouldUseSkeletalMesh != VisualFragment.bUseSkeletalMesh)
            {
                VisualFragment.bUseSkeletalMesh = bShouldUseSkeletalMesh;
                
                // In a real implementation, this would trigger a transition between vertex and skeletal animation
                // For now, just log the transition
                UE_LOG(LogTemp, Log, TEXT("Visibility: Entity %s transitioning to %s"),
                    *Context.GetEntity(EntityIndex).ToString(),
                    bShouldUseSkeletalMesh ? TEXT("skeletal mesh") : TEXT("vertex animation"));
            }
        }
    });
}

int32 UMassUnitVisibilityProcessor::DetermineVisibilityState(FMassEntityManager& EntityManager, FMassEntityHandle Entity, const FVector& ViewLocation)
{
    // Get entity view
    FMassEntityView EntityView(EntityManager, Entity);
    
    // Get transform fragment
    const FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
    
    // Calculate distance to view location
    float DistanceSquared = FVector::DistSquared(TransformFragment.GetTransform().GetLocation(), ViewLocation);
    
    // Determine LOD level based on distance
    for (int32 LODLevel = 0; LODLevel < LODDistanceThresholds.Num(); ++LODLevel)
    {
        if (DistanceSquared <= FMath::Square(LODDistanceThresholds[LODLevel]))
        {
            return LODLevel;
        }
    }
    
    // If beyond all thresholds, return max LOD level
    return LODDistanceThresholds.Num();
}
