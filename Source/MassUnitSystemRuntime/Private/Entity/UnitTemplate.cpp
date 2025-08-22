// Copyright Digi Logic Labs LLC. All Rights Reserved.

// ...existing code...
#include "Entity/UnitTemplate.h"
#include "Entity/MassUnitFragments.h"
#include "MassUnitCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Entity/MassEntityFallback.h" // For FMassUnitFragmentRequirementDescription

UUnitTemplate::UUnitTemplate()
{
    // Set default values
    UnitType = FGameplayTag::RequestGameplayTag(FName("Unit.Default"));
    UnitClass = FGameplayTag::RequestGameplayTag(FName("Class.Soldier"));
    DefaultBehavior = FGameplayTag::RequestGameplayTag(FName("Behavior.Aggressive"));
    DefaultFormation = FGameplayTag::RequestGameplayTag(FName("Formation.Line"));
    TeamFaction = FGameplayTag::RequestGameplayTag(FName("Faction.Neutral"));
}

TArray<FMassUnitFragmentRequirementDescription> UUnitTemplate::GetRequiredFragments() const
{
    TArray<FMassUnitFragmentRequirementDescription> RequiredFragments;
    // Add common fragments (custom fallback types)
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitTransformFragment>()));
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitVelocityFragment>()));
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitForceFragment>()));
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitLookAtFragment>()));

    // Add unit-specific fragments
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitStateFragment>()));
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitTargetFragment>()));
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitAbilityFragment>()));
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitTeamFragment>()));
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitVisualFragment>()));
    RequiredFragments.Add(FMassUnitFragmentRequirementDescription(StaticStruct<FMassUnitFormationFragment>()));
    return RequiredFragments;
}
