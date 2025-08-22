// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassEntityFallback.h"
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

    // Unreal override for compatibility
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    // Fallback logic for plugin independence
    void ExecuteFallback(FMassUnitEntityManagerFallback& EntityManager, FMassUnitExecutionContext& Context);
    // Query configuration
    void SetupUnitQueries();

private:
    /** Process combat interaction between attacker and target */
    void ProcessCombatInteraction(FMassUnitEntityManagerFallback& EntityManager, FMassUnitEntityHandle Attacker, FMassUnitEntityHandle Target);

    /** Entity query for processing units */
    FMassUnitEntityQuery EntityQuery;
    
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
