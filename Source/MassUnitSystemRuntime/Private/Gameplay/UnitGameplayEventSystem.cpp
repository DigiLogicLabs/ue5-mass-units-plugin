// Copyright Your Company. All Rights Reserved.

#include "Gameplay/UnitGameplayEventSystem.h"
#include "GameplayEffectTypes.h"

UUnitGameplayEventSystem::UUnitGameplayEventSystem()
{
}

UUnitGameplayEventSystem::~UUnitGameplayEventSystem()
{
}

void UUnitGameplayEventSystem::Initialize()
{
    UE_LOG(LogTemp, Log, TEXT("UnitGameplayEventSystem: Initialized"));
}

void UUnitGameplayEventSystem::Deinitialize()
{
    // Clear all listeners
    EventListeners.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("UnitGameplayEventSystem: Deinitialized"));
}

void UUnitGameplayEventSystem::DispatchEvent(FGameplayTag EventTag, FMassEntityHandle Entity, const FGameplayEventData& EventData)
{
    // Skip if event tag is invalid
    if (!EventTag.IsValid())
    {
        return;
    }
    
    // Find listeners for this event
    FOnGameplayEvent* Listeners = EventListeners.Find(EventTag);
    if (Listeners)
    {
        // Broadcast event to listeners
        Listeners->Broadcast(EventTag, Entity, EventData);
        
        UE_LOG(LogTemp, Verbose, TEXT("UnitGameplayEventSystem: Dispatched event %s for entity %s"), 
            *EventTag.ToString(), *Entity.ToString());
    }
}

void UUnitGameplayEventSystem::RegisterListener(FGameplayTag EventTag, const FOnGameplayEvent& Listener)
{
    // Skip if event tag is invalid
    if (!EventTag.IsValid())
    {
        return;
    }
    
    // Find or add listeners for this event
    FOnGameplayEvent& Listeners = EventListeners.FindOrAdd(EventTag);
    
    // Add listener
    Listeners.Add(Listener);
    
    UE_LOG(LogTemp, Log, TEXT("UnitGameplayEventSystem: Registered listener for event %s"), *EventTag.ToString());
}

void UUnitGameplayEventSystem::UnregisterListener(FGameplayTag EventTag, const FOnGameplayEvent& Listener)
{
    // Skip if event tag is invalid
    if (!EventTag.IsValid())
    {
        return;
    }
    
    // Find listeners for this event
    FOnGameplayEvent* Listeners = EventListeners.Find(EventTag);
    if (Listeners)
    {
        // Remove listener
        Listeners->Remove(Listener);
        
        UE_LOG(LogTemp, Log, TEXT("UnitGameplayEventSystem: Unregistered listener for event %s"), *EventTag.ToString());
    }
}
