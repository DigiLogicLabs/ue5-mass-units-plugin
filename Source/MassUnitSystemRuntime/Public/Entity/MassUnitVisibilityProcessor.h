// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
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

protected:
    /** Configure the processor with required fragments */
    virtual void ConfigureQueries() override;
    
    /** Execute the processor logic */
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    /** Determine visibility state for an entity */
    int32 DetermineVisibilityState(FMassEntityManager& EntityManager, FMassEntityHandle Entity, const FVector& ViewLocation);

    /** Entity query for processing units */
    FMassEntityQuery EntityQuery;
    
    /** LOD distance thresholds */
    UPROPERTY(EditAnywhere, Category = "Visibility")
    TArray<float> LODDistanceThresholds = {500.0f, 1500.0f, 3000.0f, 6000.0f};
    
    /** Maximum distance for visibility */
    UPROPERTY(EditAnywhere, Category = "Visibility")
    float MaxVisibleDistance = 10000.0f;
    
    /** Distance for skeletal mesh transition */
    UPROPERTY(EditAnywhere, Category = "Visibility")
    float SkeletalMeshDistance = 300.0f;
};
