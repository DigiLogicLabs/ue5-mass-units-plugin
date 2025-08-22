// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitVisibilityProcessor.h"
#include "MassUnitCommonFragments.h"
#include "Entity/MassUnitFragments.h"
#include "Entity/MassEntityFallback.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UMassUnitVisibilityProcessor::UMassUnitVisibilityProcessor()
{
    // ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD; // Deprecated, stubbed for independence
    LODDistanceThresholds = {500.0f, 1500.0f, 3000.0f, 6000.0f};
    MaxVisibleDistance = 10000.0f;
    SkeletalMeshDistance = 300.0f;
}

void UMassUnitVisibilityProcessor::SetupUnitQueries()
{
    EntityQuery.AddRequirement<FMassUnitTransformFragment>((int)EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassUnitVisualFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitLODFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitVisualizationLODFragment>((int)EMassFragmentAccess::ReadWrite);
}

void UMassUnitVisibilityProcessor::ExecuteFallback(FMassUnitEntityManagerFallback& EntityManager, FMassUnitExecutionContext& Context)
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
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager, ViewLocation](FMassUnitExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FMassUnitTransformFragment> TransformList = Context.GetFragmentView<FMassUnitTransformFragment>();
        const TArrayView<FMassUnitVisualFragment> VisualList = Context.GetMutableFragmentView<FMassUnitVisualFragment>();
        const TArrayView<FMassUnitLODFragment> LODList = Context.GetMutableFragmentView<FMassUnitLODFragment>();
        const TArrayView<FMassUnitVisualizationLODFragment> VisLODList = Context.GetMutableFragmentView<FMassUnitVisualizationLODFragment>();
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FMassUnitTransformFragment& TransformFragment = TransformList[EntityIndex];
            FMassUnitVisualFragment& VisualFragment = VisualList[EntityIndex];
            FMassUnitLODFragment& LODFragment = LODList[EntityIndex];
            FMassUnitVisualizationLODFragment& VisLODFragment = VisLODList[EntityIndex];
            int32 NewLODLevel = DetermineVisibilityState(EntityManager, Context.GetEntity(EntityIndex), ViewLocation);
            VisualFragment.LODLevel = NewLODLevel;
            LODFragment.Level = NewLODLevel;
            VisLODFragment.LODLevel = NewLODLevel;
            VisualFragment.bIsVisible = (NewLODLevel < LODDistanceThresholds.Num());
            float DistanceSquared = FVector::DistSquared(TransformFragment.GetTransform().GetLocation(), ViewLocation);
            bool bShouldUseSkeletalMesh = (DistanceSquared <= FMath::Square(SkeletalMeshDistance));
            if (bShouldUseSkeletalMesh != VisualFragment.bUseSkeletalMesh)
            {
                VisualFragment.bUseSkeletalMesh = bShouldUseSkeletalMesh;
                UE_LOG(LogTemp, Log, TEXT("Visibility: Entity %s transitioning to %s"),
                    *Context.GetEntity(EntityIndex).ToString(),
                    bShouldUseSkeletalMesh ? TEXT("skeletal mesh") : TEXT("vertex animation"));
            }
        }
    });
// Function body ends here
}


void UMassUnitVisibilityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Unreal override for compatibility. Redirect to fallback if needed.
    // This should be left empty or call fallback if needed.
}
int32 UMassUnitVisibilityProcessor::DetermineVisibilityState(FMassUnitEntityManagerFallback& EntityManager, FMassUnitEntityHandle Entity, const FVector& ViewLocation)
{
    FMassUnitEntityView EntityView(EntityManager, Entity);
    const FMassUnitTransformFragment& TransformFragment = EntityView.GetFragmentData<FMassUnitTransformFragment>();
    float DistanceSquared = FVector::DistSquared(TransformFragment.GetTransform().GetLocation(), ViewLocation);
    for (int32 LODLevel = 0; LODLevel < LODDistanceThresholds.Num(); ++LODLevel)
    {
        if (DistanceSquared <= FMath::Square(LODDistanceThresholds[LODLevel]))
        {
            return LODLevel;
        }
    }
    return LODDistanceThresholds.Num();
}
