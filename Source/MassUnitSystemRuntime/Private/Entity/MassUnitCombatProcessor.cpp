// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitCombatProcessor.h"
#include "MassUnitCommonFragments.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityView.h"


UMassUnitCombatProcessor::UMassUnitCombatProcessor()
{
    // ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement); // Deprecated
    // ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::LOD); // Deprecated
}

void UMassUnitCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Stub implementation for Unreal override
}

void UMassUnitCombatProcessor::SetupUnitQueries()
{
    EntityQuery.AddRequirement<FMassUnitTransformFragment>((int)EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassUnitStateFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitTargetFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitAbilityFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitTeamFragment>((int)EMassFragmentAccess::ReadOnly);
}

void UMassUnitCombatProcessor::ExecuteFallback(FMassUnitEntityManagerFallback& EntityManager, FMassUnitExecutionContext& Context)
{
    // ...existing fallback logic...
    // Function body ends here
}
void UMassUnitCombatProcessor::ProcessCombatInteraction(FMassUnitEntityManagerFallback& EntityManager, FMassUnitEntityHandle Attacker, FMassUnitEntityHandle Target)
{
    FMassUnitEntityView AttackerEntityView(EntityManager, Attacker);
    FMassUnitEntityView TargetEntityView(EntityManager, Target);
    
    // Check if both entities have required fragments
    if (!AttackerEntityView.HasFragmentData<FMassUnitAbilityFragment>() || 
        !TargetEntityView.HasFragmentData<FMassUnitAbilityFragment>() ||
        !AttackerEntityView.HasFragmentData<FMassUnitStateFragment>() || 
        !TargetEntityView.HasFragmentData<FMassUnitStateFragment>())
    {
        return;
    }
    
    // Get fragments
    const FMassUnitAbilityFragment& AttackerAbilityFragment = AttackerEntityView.GetFragmentData<FMassUnitAbilityFragment>();
    FMassUnitAbilityFragment& TargetAbilityFragment = TargetEntityView.GetFragmentData<FMassUnitAbilityFragment>();
    const FMassUnitStateFragment& AttackerStateFragment = AttackerEntityView.GetFragmentData<FMassUnitStateFragment>();
    FMassUnitStateFragment& TargetStateFragment = TargetEntityView.GetFragmentData<FMassUnitStateFragment>();
    
    // Skip if target is already dead
    if (TargetStateFragment.CurrentState == EMassUnitState::Dead)
    {
        return;
    }
    
    // Calculate damage
    float AttackerDamage = AttackerStateFragment.UnitLevel * 5.0f * DamageMultiplier; // Fallback: use level-based damage only
    
    // Apply damage to target health
    // Fallback: no health attribute, just mark as dead if damage exceeds threshold
    if (AttackerDamage > 0.0f) {
        TargetStateFragment.CurrentState = EMassUnitState::Dead;
        TargetStateFragment.StateTime = 0.0f;
        // Apply stun effect with small chance
        if (FMath::RandRange(0.0f, 1.0f) < 0.1f) {
            TargetStateFragment.StateTime = 0.0f;
        }
        UE_LOG(LogTemp, Log, TEXT("Combat: Entity %s attacked entity %s for %.1f damage"), 
            *Attacker.ToString(), *Target.ToString(), AttackerDamage);
    }
    // In a real implementation, this would trigger gameplay abilities and effects through GAS
    // For now, we're just doing a simple damage calculation
}
