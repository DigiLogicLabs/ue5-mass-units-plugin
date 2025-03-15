// Copyright Your Company. All Rights Reserved.

#include "Entity/MassUnitEntityManager.h"
#include "MassEntitySubsystem.h"
#include "Entity/UnitTemplate.h"
#include "Entity/MassUnitFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassSpawnerTypes.h"
#include "GameplayTagContainer.h"

UMassUnitEntityManager::UMassUnitEntityManager()
    : EntitySubsystem(nullptr)
{
}

UMassUnitEntityManager::~UMassUnitEntityManager()
{
}

void UMassUnitEntityManager::Initialize(UMassEntitySubsystem* InEntitySubsystem)
{
    EntitySubsystem = InEntitySubsystem;
    UE_LOG(LogTemp, Log, TEXT("MassUnitEntityManager: Initialized"));
}

FMassUnitHandle UMassUnitEntityManager::CreateUnitFromTemplate(UUnitTemplate* Template, const FTransform& SpawnTransform)
{
    return FMassUnitHandle(CreateUnitFromTemplateInternal(Template, SpawnTransform));
}

FMassEntityHandle UMassUnitEntityManager::CreateUnitFromTemplateInternal(UUnitTemplate* Template, const FTransform& SpawnTransform)
{
    if (!Template || !EntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitEntityManager: Failed to create unit - invalid template or entity subsystem"));
        return FMassEntityHandle();
    }

    // Create entity archetype with required fragments
    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    
    // Create entity with required fragments
    FMassArchetypeHandle Archetype = EntityManager.CreateArchetype(Template->GetRequiredFragments());
    FMassEntityHandle EntityHandle = EntityManager.CreateEntity(Archetype);

    // Initialize entity with template data
    FMassEntityView EntityView(EntityManager, EntityHandle);
    
    // Set transform
    if (EntityView.HasFragmentData<FTransformFragment>())
    {
        FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
        TransformFragment.SetTransform(SpawnTransform);
    }
    
    // Set unit state
    if (EntityView.HasFragmentData<FMassUnitStateFragment>())
    {
        FMassUnitStateFragment& StateFragment = EntityView.GetFragmentData<FMassUnitStateFragment>();
        StateFragment.CurrentState = EMassUnitState::Idle;
        StateFragment.StateTime = 0.0f;
        StateFragment.UnitType = Template->UnitType;
        StateFragment.UnitLevel = Template->BaseLevel;
    }
    
    // Set team
    if (EntityView.HasFragmentData<FMassUnitTeamFragment>())
    {
        FMassUnitTeamFragment& TeamFragment = EntityView.GetFragmentData<FMassUnitTeamFragment>();
        TeamFragment.TeamID = Template->TeamID;
        TeamFragment.TeamColor = Template->TeamColor;
        TeamFragment.TeamFaction = Template->TeamFaction;
    }
    
    // Set ability data
    if (EntityView.HasFragmentData<FMassUnitAbilityFragment>())
    {
        FMassUnitAbilityFragment& AbilityFragment = EntityView.GetFragmentData<FMassUnitAbilityFragment>();
        // Initialize with template abilities (will be populated later by GAS integration)
        AbilityFragment.AbilityHandles.Empty();
        AbilityFragment.ActiveEffectTags.Empty();
        
        // Initialize attributes from template
        for (const auto& AttributePair : Template->BaseAttributes)
        {
            AbilityFragment.AttributeValues.Add(AttributePair.Key, AttributePair.Value);
        }
    }
    
    // Add to type map
    UnitTypeMap.FindOrAdd(Template->UnitType).Add(EntityHandle);
    
    // Add to team map
    TeamMap.FindOrAdd(Template->TeamID).Add(EntityHandle);
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitEntityManager: Created unit of type %s for team %d"), 
        *Template->UnitType.ToString(), Template->TeamID);
    
    return EntityHandle;
}

void UMassUnitEntityManager::DestroyUnit(FMassUnitHandle UnitHandle)
{
    DestroyUnitInternal(UnitHandle.EntityHandle);
}

void UMassUnitEntityManager::DestroyUnitInternal(FMassEntityHandle EntityHandle)
{
    if (!EntitySubsystem || !EntityHandle.IsValid())
    {
        return;
    }

    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    
    // Get unit data before destroying
    FGameplayTag UnitType;
    int32 TeamID = -1;
    
    FMassEntityView EntityView(EntityManager, EntityHandle);
    if (EntityView.HasFragmentData<FMassUnitStateFragment>())
    {
        UnitType = EntityView.GetFragmentData<FMassUnitStateFragment>().UnitType;
    }
    
    if (EntityView.HasFragmentData<FMassUnitTeamFragment>())
    {
        TeamID = EntityView.GetFragmentData<FMassUnitTeamFragment>().TeamID;
    }
    
    // Remove from type map
    if (UnitType.IsValid())
    {
        if (TArray<FMassEntityHandle>* Units = UnitTypeMap.Find(UnitType))
        {
            Units->Remove(EntityHandle);
        }
    }
    
    // Remove from team map
    if (TeamID >= 0)
    {
        if (TArray<FMassEntityHandle>* Units = TeamMap.Find(TeamID))
        {
            Units->Remove(EntityHandle);
        }
    }
    
    // Destroy entity
    EntityManager.DestroyEntity(EntityHandle);
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitEntityManager: Destroyed unit"));
}

TArray<FMassUnitHandle> UMassUnitEntityManager::GetUnitsByType(FGameplayTag UnitType)
{
    TArray<FMassUnitHandle> Result;
    TArray<FMassEntityHandle> RawHandles = GetUnitsByTypeInternal(UnitType);
    
    // Convert raw handles to unit handles
    for (const FMassEntityHandle& Handle : RawHandles)
    {
        Result.Add(FMassUnitHandle(Handle));
    }
    
    return Result;
}

TArray<FMassEntityHandle> UMassUnitEntityManager::GetUnitsByTypeInternal(FGameplayTag UnitType)
{
    if (UnitType.IsValid())
    {
        if (TArray<FMassEntityHandle>* Units = UnitTypeMap.Find(UnitType))
        {
            return *Units;
        }
    }
    
    return TArray<FMassEntityHandle>();
}

TArray<FMassUnitHandle> UMassUnitEntityManager::GetUnitsByTeam(int32 TeamID)
{
    TArray<FMassUnitHandle> Result;
    TArray<FMassEntityHandle> RawHandles = GetUnitsByTeamInternal(TeamID);
    
    // Convert raw handles to unit handles
    for (const FMassEntityHandle& Handle : RawHandles)
    {
        Result.Add(FMassUnitHandle(Handle));
    }
    
    return Result;
}

TArray<FMassEntityHandle> UMassUnitEntityManager::GetUnitsByTeamInternal(int32 TeamID)
{
    if (TeamID >= 0)
    {
        if (TArray<FMassEntityHandle>* Units = TeamMap.Find(TeamID))
        {
            return *Units;
        }
    }
    
    return TArray<FMassEntityHandle>();
}
