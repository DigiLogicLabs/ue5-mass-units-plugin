// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitEntityManager.h"
#include "MassEntitySubsystem.h"
#include "Entity/UnitTemplate.h"
#include "Entity/MassUnitFragments.h"
#include "MassUnitCommonFragments.h"
#include "Entity/MassEntityFallback.h"
#include "MassSpawnerTypes.h"
#include "GameplayTagContainer.h"

UMassUnitEntityManager::UMassUnitEntityManager()
    : EntitySubsystem(nullptr)
{
}

UMassUnitEntityManager::~UMassUnitEntityManager()
{
}

void UMassUnitEntityManager::Initialize(UMassUnitEntitySubsystem* InEntitySubsystem)
{
    EntitySubsystem = InEntitySubsystem;
    UE_LOG(LogTemp, Log, TEXT("MassUnitEntityManager: Initialized"));
}

FMassUnitHandle UMassUnitEntityManager::CreateUnitFromTemplate(UUnitTemplate* Template, const FTransform& SpawnTransform)
{
    if (!Template || !EntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitEntityManager: Failed to create unit - invalid template or entity subsystem"));
        return FMassUnitHandle();
    }
    FMassUnitEntityManagerFallback& EntityManager = *EntitySubsystem->GetMutableUnitEntityManager();
    // Create entity with required fragments (custom fallback logic)
    FMassUnitEntityHandle EntityHandle = EntityManager.CreateEntity(); // Use fallback creation
    FMassUnitEntityView EntityView(EntityManager, EntityHandle);
    // Set transform
    if (EntityView.HasFragmentData<FMassUnitTransformFragment>())
    {
        FMassUnitTransformFragment& TransformFragment = EntityView.GetFragmentData<FMassUnitTransformFragment>();
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
    AbilityFragment.AbilityHandles.Empty();
    AbilityFragment.ActiveEffectTags.Empty();
    // ...existing code...
    // AttributeValues removed from FMassUnitAbilityFragment
    }
    // Add to type map
    UnitTypeMap.FindOrAdd(Template->UnitType).Add(EntityHandle);
    // Add to team map
    TeamMap.FindOrAdd(Template->TeamID).Add(EntityHandle);
    UE_LOG(LogTemp, Log, TEXT("MassUnitEntityManager: Created unit of type %s for team %d"), *Template->UnitType.ToString(), Template->TeamID);
    return FMassUnitHandle(EntityHandle);
}

void UMassUnitEntityManager::DestroyUnit(FMassUnitHandle UnitHandle)
{
    DestroyUnitInternal(UnitHandle.EntityHandle);
}

void UMassUnitEntityManager::DestroyUnitInternal(FMassUnitEntityHandle EntityHandle)
{
    if (!EntitySubsystem || !EntityHandle.IsValid())
    {
        return;
    }

    FMassUnitEntityManagerFallback& EntityManager = *EntitySubsystem->GetMutableUnitEntityManager();
    
    // Get unit data before destroying
    FGameplayTag UnitType;
    int32 TeamID = -1;
    
    FMassUnitEntityView EntityView(EntityManager, EntityHandle);
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
    if (TArray<FMassUnitEntityHandle>* Units = UnitTypeMap.Find(UnitType))
        {
            Units->Remove(EntityHandle);
        }
    }
    
    // Remove from team map
    if (TeamID >= 0)
    {
    if (TArray<FMassUnitEntityHandle>* Units = TeamMap.Find(TeamID))
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
    TArray<FMassUnitEntityHandle> RawHandles = GetUnitsByTypeInternal(UnitType);
    
    // Convert raw handles to unit handles
    for (const FMassUnitEntityHandle& Handle : RawHandles)
    {
        Result.Add(FMassUnitHandle(Handle));
    }
    
    return Result;
}

TArray<FMassUnitEntityHandle> UMassUnitEntityManager::GetUnitsByTypeInternal(FGameplayTag UnitType)
{
    if (UnitType.IsValid())
    {
    if (TArray<FMassUnitEntityHandle>* Units = UnitTypeMap.Find(UnitType))
        {
            return *Units;
        }
    }
    
    return TArray<FMassUnitEntityHandle>();
}

TArray<FMassUnitHandle> UMassUnitEntityManager::GetUnitsByTeam(int32 TeamID)
{
    TArray<FMassUnitHandle> Result;
    TArray<FMassUnitEntityHandle> RawHandles = GetUnitsByTeamInternal(TeamID);
    
    // Convert raw handles to unit handles
    for (const FMassUnitEntityHandle& Handle : RawHandles)
    {
        Result.Add(FMassUnitHandle(Handle));
    }
    
    return Result;
}

TArray<FMassUnitEntityHandle> UMassUnitEntityManager::GetUnitsByTeamInternal(int32 TeamID)
{
    if (TeamID >= 0)
    {
    if (TArray<FMassUnitEntityHandle>* Units = TeamMap.Find(TeamID))
        {
            return *Units;
        }
    }
    
    return TArray<FMassUnitEntityHandle>();
}
