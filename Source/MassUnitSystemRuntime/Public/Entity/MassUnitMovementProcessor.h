// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassUnitMovementProcessor.generated.h"

/**
 * Processor for handling unit movement in the Mass Unit System
 */
UCLASS()
class MASSUNITSYSTEMRUNTIME_API UMassUnitMovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassUnitMovementProcessor();

protected:
    /** Configure the processor with required fragments */
    virtual void ConfigureQueries() override;
    
    /** Execute the processor logic */
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    /** Calculate movement vector for an entity */
    FVector CalculateMovementVector(FMassEntityManager& EntityManager, FMassEntityHandle Entity, float DeltaTime);

    /** Entity query for processing units */
    FMassEntityQuery EntityQuery;
    
    /** Maximum speed for units */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxSpeed = 500.0f;
    
    /** Acceleration for units */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float Acceleration = 1000.0f;
    
    /** Deceleration for units */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float Deceleration = 2000.0f;
    
    /** Turning rate for units */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float TurningRate = 360.0f;
};
