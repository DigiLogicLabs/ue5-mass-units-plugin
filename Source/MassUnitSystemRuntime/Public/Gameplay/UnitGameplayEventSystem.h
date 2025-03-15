// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "UnitGameplayEventSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGameplayEvent, FGameplayTag, EventTag, FMassEntityHandle, Entity, const FGameplayEventData&, EventData);

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
    void DispatchEvent(FGameplayTag EventTag, FMassEntityHandle Entity, const FGameplayEventData& EventData);
    
    /** Register a listener for an event */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    void RegisterListener(FGameplayTag EventTag, const FOnGameplayEvent& Listener);
    
    /** Unregister a listener for an event */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    void UnregisterListener(FGameplayTag EventTag, const FOnGameplayEvent& Listener);

private:
    /** Map of event tags to listeners */
    TMap<FGameplayTag, FOnGameplayEvent> EventListeners;
};
