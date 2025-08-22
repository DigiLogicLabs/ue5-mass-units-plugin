// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "Entity/MassEntityFallback.h"
#include "MassUnitVisibilityProcessor.generated.h"

/**
 * Processor for handling unit visibility and LOD in the Mass Unit System
 */
UCLASS()
class MASSUNITSYSTEMRUNTIME_API UMassUnitVisibilityProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassUnitVisibilityProcessor();

    // Unreal override for compatibility
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    // Fallback logic for plugin independence
    void ExecuteFallback(FMassUnitEntityManagerFallback& EntityManager, FMassUnitExecutionContext& Context);
    // Query configuration
    void SetupUnitQueries();

private:
    /** Determine visibility state for an entity */
    int32 DetermineVisibilityState(FMassUnitEntityManagerFallback& EntityManager, FMassUnitEntityHandle Entity, const FVector& ViewLocation);

    /** Entity query for processing units */
    FMassUnitEntityQuery EntityQuery;

    /** LOD distance thresholds */
    UPROPERTY(EditAnywhere, Category = "Visibility")
    TArray<float> LODDistanceThresholds = {500.0f, 1500.0f, 3000.0f, 6000.0f};

    /** Maximum distance for visibility */
    UPROPERTY(EditAnywhere, Category = "Visibility")
    float MaxVisibleDistance;
    /** Distance for skeletal mesh transition */
    UPROPERTY(EditAnywhere, Category = "Visibility")
    float SkeletalMeshDistance;
};
