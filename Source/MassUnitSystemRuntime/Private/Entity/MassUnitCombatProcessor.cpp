// Copyright Your Company. All Rights Reserved.

#include "Entity/MassUnitCombatProcessor.h"
#include "MassUnitCommonFragments.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityView.h"

UMassUnitCombatProcessor::UMassUnitCombatProcessor()
{
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::LOD);
}

void UMassUnitCombatProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassUnitStateFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitTargetFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitAbilityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitTeamFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassUnitCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get delta time
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    // Process entities
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager, DeltaTime](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassUnitStateFragment> StateList = Context.GetMutableFragmentView<FMassUnitStateFragment>();
        const TArrayView<FMassUnitTargetFragment> TargetList = Context.GetMutableFragmentView<FMassUnitTargetFragment>();
        const TArrayView<FMassUnitAbilityFragment> AbilityList = Context.GetMutableFragmentView<FMassUnitAbilityFragment>();
        const TConstArrayView<FMassUnitTeamFragment> TeamList = Context.GetFragmentView<FMassUnitTeamFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& TransformFragment = TransformList[EntityIndex];
            FMassUnitStateFragment& StateFragment = StateList[EntityIndex];
            FMassUnitTargetFragment& TargetFragment = TargetList[EntityIndex];
            FMassUnitAbilityFragment& AbilityFragment = AbilityList[EntityIndex];
            const FMassUnitTeamFragment& TeamFragment = TeamList[EntityIndex];
            
            // Skip if unit is dead or stunned
            if (StateFragment.CurrentState == EMassUnitState::Dead || 
                StateFragment.CurrentState == EMassUnitState::Stunned)
            {
                continue;
            }
            
            // Check if unit has a target
            if (TargetFragment.HasTarget() && TargetFragment.TargetEntity.IsValid())
            {
                // Get target entity
                FMassEntityHandle TargetEntity = TargetFragment.TargetEntity;
                
                // Check if target is valid
                if (!EntityManager.IsEntityValid(TargetEntity))
                {
                    // Clear invalid target
                    TargetFragment.TargetEntity = FMassEntityHandle();
                    TargetFragment.TargetLocation = FVector::ZeroVector;
                    TargetFragment.TargetPriority = 0.0f;
                    
                    // Set state to idle if attacking
                    if (StateFragment.CurrentState == EMassUnitState::Attacking)
                    {
                        StateFragment.CurrentState = EMassUnitState::Idle;
                        StateFragment.StateTime = 0.0f;
                    }
                    
                    continue;
                }
                
                // Get target entity view
                FMassEntityView TargetEntityView(EntityManager, TargetEntity);
                
                // Check if target is on different team
                if (TargetEntityView.HasFragmentData<FMassUnitTeamFragment>())
                {
                    const FMassUnitTeamFragment& TargetTeamFragment = TargetEntityView.GetFragmentData<FMassUnitTeamFragment>();
                    
                    // Skip if target is on same team
                    if (TargetTeamFragment.TeamID == TeamFragment.TeamID)
                    {
                        continue;
                    }
                }
                
                // Check if target is in range
                if (TargetEntityView.HasFragmentData<FTransformFragment>())
                {
                    const FTransformFragment& TargetTransformFragment = TargetEntityView.GetFragmentData<FTransformFragment>();
                    
                    // Calculate distance to target
                    FVector ToTarget = TargetTransformFragment.GetTransform().GetLocation() - TransformFragment.GetTransform().GetLocation();
                    float DistanceToTarget = ToTarget.Size();
                    
                    // Check if in attack range
                    if (DistanceToTarget <= AttackRange)
                    {
                        // Set state to attacking if not already
                        if (StateFragment.CurrentState != EMassUnitState::Attacking)
                        {
                            StateFragment.CurrentState = EMassUnitState::Attacking;
                            StateFragment.StateTime = 0.0f;
                        }
                        
                        // Check if attack cooldown has elapsed
                        if (StateFragment.StateTime >= AttackCooldown)
                        {
                            // Process combat interaction
                            ProcessCombatInteraction(EntityManager, Context.GetEntity(EntityIndex), TargetEntity);
                            
                            // Reset attack timer
                            StateFragment.StateTime = 0.0f;
                        }
                    }
                    else
                    {
                        // Set state to moving if attacking but target out of range
                        if (StateFragment.CurrentState == EMassUnitState::Attacking)
                        {
                            StateFragment.CurrentState = EMassUnitState::Moving;
                            StateFragment.StateTime = 0.0f;
                        }
                        
                        // Update target location
                        TargetFragment.TargetLocation = TargetTransformFragment.GetTransform().GetLocation();
                    }
                }
            }
            else if (StateFragment.CurrentState == EMassUnitState::Attacking)
            {
                // Set state to idle if attacking but no target
                StateFragment.CurrentState = EMassUnitState::Idle;
                StateFragment.StateTime = 0.0f;
            }
        }
    });
}

void UMassUnitCombatProcessor::ProcessCombatInteraction(FMassEntityManager& EntityManager, FMassEntityHandle Attacker, FMassEntityHandle Target)
{
    // Get attacker entity view
    FMassEntityView AttackerEntityView(EntityManager, Attacker);
    
    // Get target entity view
    FMassEntityView TargetEntityView(EntityManager, Target);
    
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
    float AttackerDamage = 0.0f;
    if (AttackerAbilityFragment.AttributeValues.Contains(FGameplayAttribute::GetAttributeFromString("Damage")))
    {
        AttackerDamage = AttackerAbilityFragment.AttributeValues[FGameplayAttribute::GetAttributeFromString("Damage")] * DamageMultiplier;
    }
    else
    {
        // Use base damage from level if attribute not found
        AttackerDamage = AttackerStateFragment.UnitLevel * 5.0f * DamageMultiplier;
    }
    
    // Apply damage to target health
    if (TargetAbilityFragment.AttributeValues.Contains(FGameplayAttribute::GetAttributeFromString("Health")))
    {
        float& TargetHealth = TargetAbilityFragment.AttributeValues[FGameplayAttribute::GetAttributeFromString("Health")];
        TargetHealth = FMath::Max(0.0f, TargetHealth - AttackerDamage);
        
        // Check if target is dead
        if (TargetHealth <= 0.0f)
        {
            TargetStateFragment.CurrentState = EMassUnitState::Dead;
            TargetStateFragment.StateTime = 0.0f;
        }
        else
        {
            // Apply stun effect with small chance
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
            {
                TargetStateFragment.CurrentState = EMassUnitState::Stunned;
                TargetStateFragment.StateTime = 0.0f;
            }
        }
    }
    
    // In a real implementation, this would trigger gameplay abilities and effects through GAS
    // For now, we're just doing a simple damage calculation
    
    UE_LOG(LogTemp, Log, TEXT("Combat: Entity %s attacked entity %s for %.1f damage"), 
        *Attacker.ToString(), *Target.ToString(), AttackerDamage);
}
