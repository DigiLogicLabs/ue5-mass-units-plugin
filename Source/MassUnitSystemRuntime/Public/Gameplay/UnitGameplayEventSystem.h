// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Entity/MassUnitEntityManager.h"
#include "UnitGameplayEventSystem.generated.h"

// Forward declaration
struct FGameplayEventData;

// Internal delegate for gameplay events
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGameplayEvent, FGameplayTag, FMassUnitEntityHandle, const FGameplayEventData&);

/**
 * System for handling gameplay events for units in the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UUnitGameplayEventSystem : public UObject
{
    GENERATED_BODY()

public:
    UUnitGameplayEventSystem();
    virtual ~UUnitGameplayEventSystem();

    /** Initialize the event system */
    void Initialize();
    
    /** Deinitialize the event system */
    void Deinitialize();
    
    /** Dispatch an event */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    void DispatchEvent(FGameplayTag EventTag, FMassUnitHandle UnitHandle, const FGameplayEventData& EventData);

    /** Internal method to dispatch an event */
    void DispatchEventInternal(FGameplayTag EventTag, FMassUnitEntityHandle Entity, const FGameplayEventData& EventData);

    /** Register a listener for an event */
    void RegisterListener(FGameplayTag EventTag, FOnGameplayEvent::FDelegate Listener);

    /** Unregister a listener for an event */
    void UnregisterListener(FGameplayTag EventTag, FDelegateHandle ListenerHandle);
    
    /** Register a listener for an event */
    void RegisterListener(FGameplayTag EventTag, const FOnGameplayEvent& Listener);
    
    /** Unregister a listener for an event */
    void UnregisterListener(FGameplayTag EventTag, const FOnGameplayEvent& Listener);

private:
    /** Map of event tags to listeners */
    TMap<FGameplayTag, FOnGameplayEvent> EventListeners;
};
