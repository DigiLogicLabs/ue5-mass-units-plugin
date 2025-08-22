// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Entity/MassUnitEntityManager.h"
#include "Entity/MassEntityFallback.h"
#include "FormationSystem.generated.h"

/**
 * System for managing unit formations in the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UFormationSystem : public UObject
{
    GENERATED_BODY()

public:
    UFormationSystem();
    virtual ~UFormationSystem();

    /** Initialize the formation system */
    void Initialize(UMassUnitEntitySubsystem* InEntitySubsystem);
    
    /** Deinitialize the formation system */
    void Deinitialize();
    
    /** Tick function called every frame */
    void Tick(float DeltaTime);
    
    /** Create a new formation */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    int32 CreateFormation(FVector Location, FRotator Rotation, FName FormationType);
    
    /** Add an entity to a formation */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool AddEntityToFormation(FMassUnitEntityHandle Entity, int32 FormationHandle);
    
    /** Remove an entity from a formation */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool RemoveEntityFromFormation(FMassUnitEntityHandle Entity);
    
    /** Internal method to add an entity to a formation */
    bool AddEntityToFormationInternal(FMassUnitEntityHandle Entity, int32 FormationHandle);
    
    /** Internal method to remove an entity from a formation */
    bool RemoveEntityFromFormationInternal(FMassUnitEntityHandle Entity);
    
    /** Set formation target location */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool SetFormationTarget(int32 FormationHandle, FVector TargetLocation);
    
    /** Set formation shape */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool SetFormationShape(int32 FormationHandle, FName FormationShape);
    
    /** Get formation location */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    FVector GetFormationLocation(int32 FormationHandle) const;
    
    /** Get formation rotation */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    FRotator GetFormationRotation(int32 FormationHandle) const;
    
    /** Get entities in formation */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    TArray<FMassUnitEntityHandle> GetEntitiesInFormation(int32 FormationHandle) const;
    
    /** Internal method to get entities in formation */
    TArray<FMassUnitEntityHandle> GetEntitiesInFormationInternal(int32 FormationHandle) const;

private:
    /** Reference to the Mass Entity Subsystem */
    UPROPERTY(Transient)
    UMassUnitEntitySubsystem* EntitySubsystem;
    
    /** Structure for formation data */
    struct FFormationData
    {
        FVector Location;
        FRotator Rotation;
        FVector TargetLocation;
        FName FormationType;
        FName FormationShape;
    TArray<FMassUnitEntityHandle> Entities;
    TMap<FMassUnitEntityHandle, int32> EntitySlots;
        float FormationWidth;
        float FormationDepth;
        float UnitSpacing;
        bool bIsMoving;
        
        FFormationData()
            : Location(FVector::ZeroVector)
            , Rotation(FRotator::ZeroRotator)
            , TargetLocation(FVector::ZeroVector)
            , FormationType(NAME_None)
            , FormationShape(NAME_None)
            , FormationWidth(1000.0f)
            , FormationDepth(1000.0f)
            , UnitSpacing(150.0f)
            , bIsMoving(false)
        {
        }
    };
    
    /** Map of formation handles to formation data */
    TMap<int32, FFormationData> Formations;
    
    /** Map of entity handles to formation handles */
    TMap<FMassUnitEntityHandle, int32> EntityFormationMap;
    
    /** Next formation handle */
    int32 NextFormationHandle;
    
    /** Update formation positions */
    void UpdateFormationPositions(float DeltaTime);
    
    /** Update formation movement */
    void UpdateFormationMovement(float DeltaTime);
    
    /** Calculate slot position in formation */
    FVector CalculateSlotPosition(const FFormationData& Formation, int32 SlotIndex);
    
    /** Assign slot to entity */
    int32 AssignSlotToEntity(FFormationData& Formation, FMassUnitEntityHandle Entity);
    
    /** Update entity formation data */
    void UpdateEntityFormationData(FMassUnitEntityHandle Entity, int32 FormationHandle, int32 SlotIndex);
};
