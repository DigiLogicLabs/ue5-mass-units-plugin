// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/UnitTemplate.h"

#include "Core/MassUnitGameplayTags.h"
#include "Entity/MassUnitFragments.h"
#include "MassUnitCommonFragments.h"

UUnitTemplate::UUnitTemplate()
{
	using namespace UE::MassUnitSystem;
	UnitType = Tags::UnitTypeDefault();
	UnitClass = Tags::UnitClassSoldier();
	DefaultBehavior = Tags::BehaviorIdle();
	DefaultFormation = Tags::FormationLine();
	TeamFaction = Tags::FactionNeutral();
	AnimationTags = {
		Tags::AnimationIdle(),
		Tags::AnimationWalk(),
		Tags::AnimationRun(),
		Tags::AnimationAttack(),
		Tags::AnimationDeath(),
		Tags::AnimationStun()
	};
}

TArray<const UScriptStruct*> UUnitTemplate::GetRequiredFragments() const
{
	return {
		FMassUnitTransformFragment::StaticStruct(),
		FMassUnitVelocityFragment::StaticStruct(),
		FMassUnitForceFragment::StaticStruct(),
		FMassUnitLookAtFragment::StaticStruct(),
		FMassUnitStateFragment::StaticStruct(),
		FMassUnitTargetFragment::StaticStruct(),
		FMassUnitAbilityFragment::StaticStruct(),
		FMassUnitTeamFragment::StaticStruct(),
		FMassUnitVisualFragment::StaticStruct(),
		FMassUnitFormationFragment::StaticStruct(),
		FMassUnitNavigationFragment::StaticStruct(),
		FMassUnitCrowdFragment::StaticStruct(),
		FMassUnitLODFragment::StaticStruct(),
		FMassUnitVisualizationLODFragment::StaticStruct()
	};
}
