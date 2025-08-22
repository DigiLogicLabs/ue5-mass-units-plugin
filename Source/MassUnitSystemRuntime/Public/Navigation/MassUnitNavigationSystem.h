// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Entity/MassUnitEntityManager.h"
#include "Entity/MassEntityFallback.h"
#include "MassUnitNavigationSystem.generated.h"
class UNavigationSystemV1;
class ANavigationData;

/**
 * Custom navigation system for mass units in the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UMassUnitNavigationSystem : public UObject
{
    GENERATED_BODY()

public:
    UMassUnitNavigationSystem();
    virtual ~UMassUnitNavigationSystem();

    /** Initialize the navigation system */
    void Initialize(UWorld* InWorld, UMassUnitEntitySubsystem* InEntitySubsystem);
    
    /** Deinitialize the navigation system */
    void Deinitialize();
    
    /** Update navigation data */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    void UpdateNavigationData(UWorld* World);
    
    /** Request a path for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    // bool RequestPathInternal(FMassUnitEntityHandle Entity, const FVector& Destination); // Remove duplicate declaration
    
    /** Internal method to request a path for an entity */
    bool RequestPathInternal(FMassUnitEntityHandle Entity, const FVector& Destination);
    
    /** Process path requests */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    void ProcessPathRequests();
    
    /** Get the navigation system */
    UNavigationSystemV1* GetNavigationSystem() const { return NavigationSystem; }

private:
    /** Reference to the world */
    UPROPERTY(Transient)
    UWorld* World;
    
    /** Reference to the Mass Entity Subsystem */
    UPROPERTY(Transient)
    UMassUnitEntitySubsystem* EntitySubsystem;
    
    /** Reference to the navigation system */
    UPROPERTY(Transient)
    UNavigationSystemV1* NavigationSystem;
    
    /** Reference to the navigation data */
    UPROPERTY(Transient)
    ANavigationData* NavigationData;
    
    /** Structure for path requests */
    struct FPathRequest
    {
    FMassUnitEntityHandle Entity;
        FVector Destination;
        TSharedPtr<FNavPathSharedPtr> ResultPath;
        FNavPathQueryDelegate Delegate;
    };
    
    /** Queue of path requests */
    TArray<FPathRequest> PathRequestQueue;
    
    /** Map of entity handles to path results */
    TMap<FMassUnitEntityHandle, FNavPathSharedPtr> EntityPathMap;
    
    /** Maximum number of path requests to process per frame */
    UPROPERTY(EditAnywhere, Category = "Navigation")
    int32 MaxPathRequestsPerFrame = 100;
    
    /** Path request batch size */
    UPROPERTY(EditAnywhere, Category = "Navigation")
    int32 PathRequestBatchSize = 10;
    
    /** Process a batch of path requests */
    void ProcessPathRequestBatch(int32 BatchSize);
    
    /** Handle path request completion */
    void HandlePathRequestComplete(uint32 PathId, ENavigationQueryResult::Type Result, FNavPathSharedPtr Path);
    
    /** Update entity with path result */
    void UpdateEntityWithPath(FMassUnitEntityHandle Entity, const FNavPathSharedPtr& Path);
};
