// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassUnitCombatProcessor.generated.h"

/**
 * Processor for handling unit combat interactions in the Mass Unit System
 */
UCLASS()
class MASSUNITSYSTEMRUNTIME_API UMassUnitCombatProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassUnitCombatProcessor();

protected:
    /** Configure the processor with required fragments */
    virtual void ConfigureQueries() override;
    
    /** Execute the processor logic */
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    /** Process combat interaction between attacker and target */
    void ProcessCombatInteraction(FMassEntityManager& EntityManager, FMassEntityHandle Attacker, FMassEntityHandle Target);

    /** Entity query for processing units */
    FMassEntityQuery EntityQuery;
    
    /** Attack range for units */
    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackRange = 200.0f;
    
    /** Attack cooldown time in seconds */
    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackCooldown = 1.0f;
    
    /** Base damage multiplier */
    UPROPERTY(EditAnywhere, Category = "Combat")
    float DamageMultiplier = 1.0f;
};
