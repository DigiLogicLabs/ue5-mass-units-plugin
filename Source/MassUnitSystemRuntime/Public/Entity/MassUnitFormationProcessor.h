// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassEntityFallback.h"
#include "MassProcessor.h"
#include "MassUnitFormationProcessor.generated.h"

/**
 * Processor for handling unit formations in the Mass Unit System
 */
UCLASS()
class MASSUNITSYSTEMRUNTIME_API UMassUnitFormationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassUnitFormationProcessor();

    // Unreal override for compatibility
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    // Fallback logic for plugin independence
    void ExecuteFallback(FMassUnitEntityManagerFallback& EntityManager, FMassUnitExecutionContext& Context);
    // Query configuration
    void SetupUnitQueries();

private:
    /** Calculate formation position for an entity */
    FVector CalculateFormationPosition(FMassUnitEntityManagerFallback& EntityManager, FMassUnitEntityHandle Entity, int32 FormationHandle);

    /** Entity query for processing units in formations */
    FMassUnitEntityQuery EntityQuery;
    
    /** Formation spacing */
    UPROPERTY(EditAnywhere, Category = "Formation")
    float UnitSpacing = 150.0f;
    
    /** Formation cohesion strength */
    UPROPERTY(EditAnywhere, Category = "Formation")
    float CohesionStrength = 1.0f;
    
    /** Formation shape adaptation rate */
    UPROPERTY(EditAnywhere, Category = "Formation")
    float AdaptationRate = 0.5f;
};
