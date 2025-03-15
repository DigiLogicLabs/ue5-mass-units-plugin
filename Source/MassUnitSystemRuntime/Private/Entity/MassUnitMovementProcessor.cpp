// Copyright Your Company. All Rights Reserved.

#include "Entity/MassUnitMovementProcessor.h"
#include "MassCommonFragments.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityView.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"

UMassUnitMovementProcessor::UMassUnitMovementProcessor()
{
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::LOD);
}

void UMassUnitMovementProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassLookAtFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitStateFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassUnitTargetFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassUnitFormationFragment>(EMassFragmentAccess::ReadOnly);
    
    // Optional fragments
    EntityQuery.AddTagRequirement<FMassNavigationFragmentTag>(EMassFragmentPresence::Optional);
}

void UMassUnitMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get delta time
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    // Process entities
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager, DeltaTime](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FMassForceFragment> ForceList = Context.GetMutableFragmentView<FMassForceFragment>();
        const TArrayView<FMassLookAtFragment> LookAtList = Context.GetMutableFragmentView<FMassLookAtFragment>();
        const TArrayView<FMassUnitStateFragment> StateList = Context.GetMutableFragmentView<FMassUnitStateFragment>();
        const TConstArrayView<FMassUnitTargetFragment> TargetList = Context.GetFragmentView<FMassUnitTargetFragment>();
        const TConstArrayView<FMassUnitFormationFragment> FormationList = Context.GetFragmentView<FMassUnitFormationFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& TransformFragment = TransformList[EntityIndex];
            FMassVelocityFragment& VelocityFragment = VelocityList[EntityIndex];
            FMassForceFragment& ForceFragment = ForceList[EntityIndex];
            FMassLookAtFragment& LookAtFragment = LookAtList[EntityIndex];
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

FVector UMassUnitMovementProcessor::CalculateMovementVector(FMassEntityManager& EntityManager, FMassEntityHandle Entity, float DeltaTime)
{
    FVector MovementVector = FVector::ZeroVector;
    
    // Get entity view
    FMassEntityView EntityView(EntityManager, Entity);
    
    // Get fragments
    const FMassUnitTargetFragment& TargetFragment = EntityView.GetFragmentData<FMassUnitTargetFragment>();
    const FMassUnitFormationFragment& FormationFragment = EntityView.GetFragmentData<FMassUnitFormationFragment>();
    const FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
    
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
