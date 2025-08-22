// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitMovementProcessor.h"
#include "MassUnitCommonFragments.h"
#include "Entity/MassUnitFragments.h"


UMassUnitMovementProcessor::UMassUnitMovementProcessor()
{
    // ExecutionOrder can be set to custom values if needed
}

void UMassUnitMovementProcessor::SetupUnitQueries()
{
    EntityQuery.AddRequirement<FMassUnitTransformFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitVelocityFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitForceFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitLookAtFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitStateFragment>((int)EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitTargetFragment>((int)EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassUnitFormationFragment>((int)EMassFragmentAccess::ReadOnly);
    
    // Optional fragments
    EntityQuery.AddTagRequirement<int>((int)EMassFragmentPresence::Optional);
}

void UMassUnitMovementProcessor::ExecuteFallback(FMassUnitEntityManagerFallback& EntityManager, FMassUnitExecutionContext& Context)
{
    // Get delta time
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    // Process entities
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager, DeltaTime](FMassUnitExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
    const TArrayView<FMassUnitTransformFragment> TransformList = Context.GetMutableFragmentView<FMassUnitTransformFragment>();
    const TArrayView<FMassUnitVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassUnitVelocityFragment>();
    const TArrayView<FMassUnitForceFragment> ForceList = Context.GetMutableFragmentView<FMassUnitForceFragment>();
    const TArrayView<FMassUnitLookAtFragment> LookAtList = Context.GetMutableFragmentView<FMassUnitLookAtFragment>();
        const TArrayView<FMassUnitStateFragment> StateList = Context.GetMutableFragmentView<FMassUnitStateFragment>();
        const TConstArrayView<FMassUnitTargetFragment> TargetList = Context.GetFragmentView<FMassUnitTargetFragment>();
        const TConstArrayView<FMassUnitFormationFragment> FormationList = Context.GetFragmentView<FMassUnitFormationFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FMassUnitTransformFragment& TransformFragment = TransformList[EntityIndex];
            FMassUnitVelocityFragment& VelocityFragment = VelocityList[EntityIndex];
            FMassUnitForceFragment& ForceFragment = ForceList[EntityIndex];
            FMassUnitLookAtFragment& LookAtFragment = LookAtList[EntityIndex];
            FMassUnitStateFragment& StateFragment = StateList[EntityIndex];
            const FMassUnitTargetFragment& TargetFragment = TargetList[EntityIndex];
            const FMassUnitFormationFragment& FormationFragment = FormationList[EntityIndex];
            
            // Update state time
            StateFragment.StateTime += DeltaTime;
            
            // Skip if unit is dead or stunned
            if (StateFragment.CurrentState == EMassUnitState::Dead || 
                StateFragment.CurrentState == EMassUnitState::Stunned)
            {
                VelocityFragment.Value = FVector::ZeroVector;
                ForceFragment.Value = FVector::ZeroVector;
                continue;
            }
            
            // Calculate movement vector
            FVector MovementVector = CalculateMovementVector(EntityManager, Context.GetEntity(EntityIndex), DeltaTime);
            
            // Apply movement
            if (!MovementVector.IsZero())
            {
                // Set state to moving if not attacking or interacting
                if (StateFragment.CurrentState != EMassUnitState::Attacking && 
                    StateFragment.CurrentState != EMassUnitState::Interacting)
                {
                    StateFragment.CurrentState = EMassUnitState::Moving;
                }
                
                // Apply force
                ForceFragment.Value = MovementVector * Acceleration;
                
                // Clamp velocity
                if (VelocityFragment.Value.SizeSquared() > FMath::Square(MaxSpeed))
                {
                    VelocityFragment.Value = VelocityFragment.Value.GetSafeNormal() * MaxSpeed;
                }
                
                // Update look at direction
                LookAtFragment.Direction = VelocityFragment.Value.GetSafeNormal();
            }
            else
            {
                // Apply deceleration
                if (!VelocityFragment.Value.IsZero())
                {
                    FVector DecelDir = -VelocityFragment.Value.GetSafeNormal();
                    float DecelAmount = Deceleration * DeltaTime;
                    float CurrentSpeed = VelocityFragment.Value.Size();
                    
                    if (CurrentSpeed <= DecelAmount)
                    {
                        VelocityFragment.Value = FVector::ZeroVector;
                        ForceFragment.Value = FVector::ZeroVector;
                        
                        // Set state to idle if moving
                        if (StateFragment.CurrentState == EMassUnitState::Moving)
                        {
                            StateFragment.CurrentState = EMassUnitState::Idle;
                            StateFragment.StateTime = 0.0f;
                        }
                    }
                    else
                    {
                        ForceFragment.Value = DecelDir * Deceleration;
                    }
                }
                else if (StateFragment.CurrentState == EMassUnitState::Moving)
                {
                    // Set state to idle if moving but no velocity
                    StateFragment.CurrentState = EMassUnitState::Idle;
                    StateFragment.StateTime = 0.0f;
                }
            }
        }
    });
}

void UMassUnitMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Unreal override for compatibility. Redirect to fallback if needed.
}

FVector UMassUnitMovementProcessor::CalculateMovementVector(FMassUnitEntityManagerFallback& EntityManager, FMassUnitEntityHandle Entity, float DeltaTime)
{
    FVector MovementVector = FVector::ZeroVector;
    
    // Get entity view
    FMassUnitEntityView EntityView(EntityManager, Entity);
    
    // Get fragments
    const FMassUnitTargetFragment& TargetFragment = EntityView.GetFragmentData<FMassUnitTargetFragment>();
    const FMassUnitFormationFragment& FormationFragment = EntityView.GetFragmentData<FMassUnitFormationFragment>();
    const FMassUnitTransformFragment& TransformFragment = EntityView.GetFragmentData<FMassUnitTransformFragment>();
    
    // Priority 1: Formation movement
    if (FormationFragment.IsInFormation())
    {
        // Calculate formation position (in a real implementation, this would come from the formation system)
        FVector FormationPosition = TransformFragment.GetTransform().GetLocation() + FormationFragment.FormationOffset;
        FVector ToFormationPosition = FormationPosition - TransformFragment.GetTransform().GetLocation();
        
        // If not at formation position, move towards it
        if (ToFormationPosition.SizeSquared() > 1.0f)
        {
            MovementVector = ToFormationPosition.GetSafeNormal();
            return MovementVector;
        }
    }
    
    // Priority 2: Target movement
    if (TargetFragment.HasTarget())
    {
        if (TargetFragment.TargetEntity.IsValid())
        {
            // Move towards target entity (in a real implementation, this would get the target entity's position)
            // For now, just use the target location
            FVector ToTarget = TargetFragment.TargetLocation - TransformFragment.GetTransform().GetLocation();
            
            // If not at target, move towards it
            if (ToTarget.SizeSquared() > 100.0f) // Stop within 10 units of target
            {
                MovementVector = ToTarget.GetSafeNormal();
                return MovementVector;
            }
        }
        else if (!TargetFragment.TargetLocation.IsZero())
        {
            // Move towards target location
            FVector ToTarget = TargetFragment.TargetLocation - TransformFragment.GetTransform().GetLocation();
            
            // If not at target, move towards it
            if (ToTarget.SizeSquared() > 100.0f) // Stop within 10 units of target
            {
                MovementVector = ToTarget.GetSafeNormal();
                return MovementVector;
            }
        }
    }
    
    // No movement needed
    return MovementVector;
}
