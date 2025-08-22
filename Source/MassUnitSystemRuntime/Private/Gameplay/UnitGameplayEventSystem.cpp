// Copyright Digi Logic Labs LLC. All Rights Reserved.

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

void UUnitGameplayEventSystem::DispatchEvent(FGameplayTag EventTag, FMassUnitHandle UnitHandle, const FGameplayEventData& EventData)
{
    // Convert UnitHandle to FMassUnitEntityHandle using direct member access
    FMassUnitEntityHandle Entity = UnitHandle.EntityHandle;
    DispatchEventInternal(EventTag, Entity, EventData);
}

void UUnitGameplayEventSystem::DispatchEventInternal(FGameplayTag EventTag, FMassUnitEntityHandle Entity, const FGameplayEventData& EventData)
{
    if (!EventTag.IsValid())
    {
        return;
    }
    FOnGameplayEvent* Listeners = EventListeners.Find(EventTag);
    if (Listeners)
    {
        Listeners->Broadcast(EventTag, Entity, EventData);
        UE_LOG(LogTemp, Verbose, TEXT("UnitGameplayEventSystem: Dispatched event %s for entity %s"), *EventTag.ToString(), *Entity.ToString());
    }
}

void UUnitGameplayEventSystem::RegisterListener(FGameplayTag EventTag, FOnGameplayEvent::FDelegate Listener)
{
    if (!EventTag.IsValid())
    {
        return;
    }
    FOnGameplayEvent& Listeners = EventListeners.FindOrAdd(EventTag);
    Listeners.Add(Listener);
    UE_LOG(LogTemp, Log, TEXT("UnitGameplayEventSystem: Registered listener for event %s"), *EventTag.ToString());
}

void UUnitGameplayEventSystem::UnregisterListener(FGameplayTag EventTag, FDelegateHandle ListenerHandle)
{
    if (!EventTag.IsValid())
    {
        return;
    }
    FOnGameplayEvent* Listeners = EventListeners.Find(EventTag);
    if (Listeners)
    {
        Listeners->Remove(ListenerHandle);
        UE_LOG(LogTemp, Log, TEXT("UnitGameplayEventSystem: Unregistered listener for event %s"), *EventTag.ToString());
    }
}
