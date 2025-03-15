// Copyright Your Company. All Rights Reserved.

#include "Core/MassUnitSubsystem.h"
#include "MassEntitySubsystem.h"
#include "Entity/MassUnitEntityManager.h"
#include "Navigation/FormationSystem.h"
#include "Navigation/MassUnitNavigationSystem.h"
#include "Visual/NiagaraUnitSystem.h"
#include "Visual/UnitMeshPool.h"
#include "Gameplay/GASUnitIntegration.h"
#include "Gameplay/GASCompanionIntegration.h"
#include "Gameplay/UnitGameplayEventSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMassUnitSubsystem::UMassUnitSubsystem()
    : EntitySubsystem(nullptr)
    , UnitManager(nullptr)
    , FormationSystem(nullptr)
    , NavigationSystem(nullptr)
    , NiagaraSystem(nullptr)
    , MeshPool(nullptr)
    , GASIntegration(nullptr)
    , GASCompanionIntegration(nullptr)
    , GameplayEventSystem(nullptr)
{
}

UMassUnitSubsystem::~UMassUnitSubsystem()
{
}

void UMassUnitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    // Get the Mass Entity Subsystem
    EntitySubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
    if (!EntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to get MassEntitySubsystem"));
        return;
    }
    
    // Get the world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to get World"));
        return;
    }
    
    // Create the Unit Manager
    UnitManager = NewObject<UMassUnitEntityManager>(this);
    if (UnitManager)
    {
        UnitManager->Initialize(EntitySubsystem);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create UnitManager"));
    }
    
    // Create the Formation System
    FormationSystem = NewObject<UFormationSystem>(this);
    if (FormationSystem)
    {
        FormationSystem->Initialize(EntitySubsystem);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create FormationSystem"));
    }
    
    // Create the Navigation System
    NavigationSystem = NewObject<UMassUnitNavigationSystem>(this);
    if (NavigationSystem)
    {
        NavigationSystem->Initialize(World, EntitySubsystem);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create NavigationSystem"));
    }
    
    // Create the Niagara System
    NiagaraSystem = NewObject<UNiagaraUnitSystem>(this);
    if (NiagaraSystem)
    {
        NiagaraSystem->Initialize(World, EntitySubsystem);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create NiagaraSystem"));
    }
    
    // Create the Mesh Pool
    MeshPool = NewObject<UUnitMeshPool>(this);
    if (MeshPool)
    {
        MeshPool->Initialize(World, EntitySubsystem, 100); // Default pool size of 100
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create MeshPool"));
    }
    
    // Create the GAS Integration
    GASIntegration = NewObject<UGASUnitIntegration>(this);
    if (GASIntegration)
    {
        GASIntegration->Initialize(EntitySubsystem);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create GASIntegration"));
    }
    
    // Create the GASCompanion Integration
    GASCompanionIntegration = NewObject<UGASCompanionIntegration>(this);
    if (GASCompanionIntegration && GASIntegration)
    {
        GASCompanionIntegration->Initialize(GASIntegration);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create GASCompanionIntegration"));
    }
    
    // Create the Gameplay Event System
    GameplayEventSystem = NewObject<UUnitGameplayEventSystem>(this);
    if (GameplayEventSystem)
    {
        GameplayEventSystem->Initialize();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create GameplayEventSystem"));
    }
    
    // Register tick function
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        TickDelegateHandle = GameInstance->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateUObject(this, &UMassUnitSubsystem::Tick, 0.0f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitSubsystem: Initialized"));
}

void UMassUnitSubsystem::Deinitialize()
{
    // Unregister tick function
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        GameInstance->GetTimerManager().ClearTimer(TickDelegateHandle);
    }
    
    // Clean up Gameplay Event System
    if (GameplayEventSystem)
    {
        GameplayEventSystem->Deinitialize();
        GameplayEventSystem = nullptr;
    }
    
    // Clean up GASCompanion Integration
    if (GASCompanionIntegration)
    {
        GASCompanionIntegration->Deinitialize();
        GASCompanionIntegration = nullptr;
    }
    
    // Clean up GAS Integration
    if (GASIntegration)
    {
        GASIntegration->Deinitialize();
        GASIntegration = nullptr;
    }
    
    // Clean up Mesh Pool
    if (MeshPool)
    {
        MeshPool->Deinitialize();
        MeshPool = nullptr;
    }
    
    // Clean up Niagara System
    if (NiagaraSystem)
    {
        NiagaraSystem->Deinitialize();
        NiagaraSystem = nullptr;
    }
    
    // Clean up Navigation System
    if (NavigationSystem)
    {
        NavigationSystem->Deinitialize();
        NavigationSystem = nullptr;
    }
    
    // Clean up Formation System
    if (FormationSystem)
    {
        FormationSystem->Deinitialize();
        FormationSystem = nullptr;
    }
    
    // Clean up Unit Manager
    if (UnitManager)
    {
        UnitManager = nullptr;
    }
    
    // Clean up Entity Subsystem reference
    EntitySubsystem = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitSubsystem: Deinitialized"));
}

void UMassUnitSubsystem::Tick(float DeltaTime)
{
    // Register for next tick
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        TickDelegateHandle = GameInstance->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateUObject(this, &UMassUnitSubsystem::Tick, 0.0f));
    }
    
    // Update Formation System
    if (FormationSystem)
    {
        FormationSystem->Tick(DeltaTime);
    }
    
    // Process navigation path requests
    if (NavigationSystem)
    {
        NavigationSystem->ProcessPathRequests();
    }
    
    // Update unit visuals
    if (NiagaraSystem && UnitManager)
    {
        // Get all units
        TArray<FMassEntityHandle> AllUnits;
        for (auto& Pair : UnitManager->GetUnitTypeMap())
        {
            AllUnits.Append(Pair.Value);
        }
        
        // Update visuals for units
        NiagaraSystem->UpdateUnitVisualsInternal(AllUnits);
    }
}

UMassUnitEntityManager* UMassUnitSubsystem::GetUnitManager() const
{
    return UnitManager;
}

UFormationSystem* UMassUnitSubsystem::GetFormationSystem() const
{
    return FormationSystem;
}

UMassUnitNavigationSystem* UMassUnitSubsystem::GetNavigationSystem() const
{
    return NavigationSystem;
}

UNiagaraUnitSystem* UMassUnitSubsystem::GetNiagaraSystem() const
{
    return NiagaraSystem;
}

UUnitMeshPool* UMassUnitSubsystem::GetMeshPool() const
{
    return MeshPool;
}

UGASUnitIntegration* UMassUnitSubsystem::GetGASIntegration() const
{
    return GASIntegration;
}

UGASCompanionIntegration* UMassUnitSubsystem::GetGASCompanionIntegration() const
{
    return GASCompanionIntegration;
}

UUnitGameplayEventSystem* UMassUnitSubsystem::GetGameplayEventSystem() const
{
    return GameplayEventSystem;
}
