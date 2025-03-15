// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

protected:
    /** Configure the processor with required fragments */
    virtual void ConfigureQueries() override;
    
    /** Execute the processor logic */
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    /** Calculate formation position for an entity */
    FVector CalculateFormationPosition(FMassEntityManager& EntityManager, FMassEntityHandle Entity, int32 FormationHandle);

    /** Entity query for processing units in formations */
    FMassEntityQuery EntityQuery;
    
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
