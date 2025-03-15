// Copyright Your Company. All Rights Reserved.

#include "Entity/UnitTemplate.h"
#include "Entity/MassUnitFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"

UUnitTemplate::UUnitTemplate()
{
    // Set default values
    UnitType = FGameplayTag::RequestGameplayTag(FName("Unit.Default"));
    UnitClass = FGameplayTag::RequestGameplayTag(FName("Class.Soldier"));
    DefaultBehavior = FGameplayTag::RequestGameplayTag(FName("Behavior.Aggressive"));
    DefaultFormation = FGameplayTag::RequestGameplayTag(FName("Formation.Line"));
    TeamFaction = FGameplayTag::RequestGameplayTag(FName("Faction.Neutral"));
}

TArray<FMassFragmentRequirementDescription> UUnitTemplate::GetRequiredFragments() const
{
    TArray<FMassFragmentRequirementDescription> RequiredFragments;
    
    // Add common fragments
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FTransformFragment::StaticStruct() });
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassVelocityFragment::StaticStruct() });
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassForceFragment::StaticStruct() });
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassLookAtFragment::StaticStruct() });
    
    // Add unit-specific fragments
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassUnitStateFragment::StaticStruct() });
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassUnitTargetFragment::StaticStruct() });
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassUnitAbilityFragment::StaticStruct() });
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassUnitTeamFragment::StaticStruct() });
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassUnitVisualFragment::StaticStruct() });
    RequiredFragments.Add(FMassFragmentRequirementDescription{ FMassUnitFormationFragment::StaticStruct() });
    
    return RequiredFragments;
}
