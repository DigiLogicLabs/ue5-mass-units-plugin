// Copyright Your Company. All Rights Reserved.

#include "Core/MassUnitSubsystem.h"
#include "MassEntitySubsystem.h"
#include "Entity/MassUnitEntityManager.h"
#include "Navigation/FormationSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UMassUnitSubsystem::UMassUnitSubsystem()
    : EntitySubsystem(nullptr)
    , UnitManager(nullptr)
    , FormationSystem(nullptr)
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
        // Initialize formation system (implementation would be in FormationSystem.cpp)
        // FormationSystem->Initialize();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitSubsystem: Failed to create FormationSystem"));
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
    
    // Clean up Formation System
    if (FormationSystem)
    {
        // Deinitialize formation system (implementation would be in FormationSystem.cpp)
        // FormationSystem->Deinitialize();
        FormationSystem = nullptr;
    }
    
    // Clean up Unit Manager
    UnitManager = nullptr;
    
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
        // Tick formation system (implementation would be in FormationSystem.cpp)
        // FormationSystem->Tick(DeltaTime);
    }
    
    // Additional subsystem-level updates can be performed here
}
