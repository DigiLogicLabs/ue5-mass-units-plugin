// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitCombatProcessor.h"

#include "Core/MassUnitGameplayTags.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"

UMassUnitCombatProcessor::UMassUnitCombatProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = FName(TEXT("MassUnitSystem.Combat"));
	ExecutionOrder.ExecuteAfter.Add(FName(TEXT("MassUnitSystem.Movement")));
}

void UMassUnitCombatProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassUnitTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassUnitStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassUnitTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassUnitTeamFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassUnitVisualFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassUnitCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(Context.GetDeltaTimeSeconds(), 0.1f);
	EntityQuery.ForEachEntityChunk(Context, [this, &EntityManager, DeltaTime](FMassExecutionContext& ChunkContext)
	{
		const TConstArrayView<FMassUnitTransformFragment> Transforms = ChunkContext.GetFragmentView<FMassUnitTransformFragment>();
		TArrayView<FMassUnitStateFragment> States = ChunkContext.GetMutableFragmentView<FMassUnitStateFragment>();
		TArrayView<FMassUnitTargetFragment> Targets = ChunkContext.GetMutableFragmentView<FMassUnitTargetFragment>();
		const TConstArrayView<FMassUnitTeamFragment> Teams = ChunkContext.GetFragmentView<FMassUnitTeamFragment>();
		TArrayView<FMassUnitVisualFragment> Visuals = ChunkContext.GetMutableFragmentView<FMassUnitVisualFragment>();

		for (FMassExecutionContext::FEntityIterator It = ChunkContext.CreateEntityIterator(); It; ++It)
		{
			FMassUnitStateFragment& State = States[It];
			FMassUnitTargetFragment& Target = Targets[It];
			FMassUnitVisualFragment& Visual = Visuals[It];
			State.AttackCooldownRemaining = FMath::Max(0.0f, State.AttackCooldownRemaining - DeltaTime);

			if (State.CurrentState == EMassUnitState::Dead || !Target.TargetEntity.IsValid())
			{
				continue;
			}

			const FMassEntityHandle TargetHandle = Target.TargetEntity.ToMassEntityHandle();
			if (!EntityManager.IsEntityValid(TargetHandle))
			{
				Target.Clear();
				continue;
			}

			FMassUnitStateFragment* TargetState = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(TargetHandle);
			const FMassUnitTransformFragment* TargetTransform = EntityManager.GetFragmentDataPtr<FMassUnitTransformFragment>(TargetHandle);
			const FMassUnitTeamFragment* TargetTeam = EntityManager.GetFragmentDataPtr<FMassUnitTeamFragment>(TargetHandle);
			if (!TargetState || !TargetTransform || !TargetTeam || TargetState->CurrentState == EMassUnitState::Dead || TargetTeam->TeamID == Teams[It].TeamID)
			{
				if (TargetState && TargetState->CurrentState == EMassUnitState::Dead)
				{
					Target.Clear();
				}
				continue;
			}

			Target.TargetLocation = TargetTransform->GetTransform().GetLocation();
			const float DistanceSquared = FVector::DistSquared2D(Transforms[It].GetTransform().GetLocation(), Target.TargetLocation);
			if (DistanceSquared > FMath::Square(State.AttackRange))
			{
				continue;
			}

			if (State.CurrentState != EMassUnitState::Attacking)
			{
				State.CurrentState = EMassUnitState::Attacking;
				State.StateTime = 0.0f;
			}
			Visual.CurrentAnimation = UE::MassUnitSystem::Tags::AnimationAttack();

			if (State.AttackCooldownRemaining <= 0.0f)
			{
				const float Damage = State.BaseDamage * FMath::Max(1, State.UnitLevel) * DamageMultiplier;
				TargetState->Health = FMath::Max(0.0f, TargetState->Health - Damage);
				State.AttackCooldownRemaining = State.AttackCooldown;
				if (TargetState->Health <= 0.0f)
				{
					TargetState->CurrentState = EMassUnitState::Dead;
					TargetState->StateTime = 0.0f;
					Target.Clear();
				}
			}
		}
	});
}
