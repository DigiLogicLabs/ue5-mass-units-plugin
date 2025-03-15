// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "UnitMeshPool.generated.h"

class USkeletalMeshComponent;
class UMassEntitySubsystem;

/**
 * Pool of skeletal meshes for player-interactive units in the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UUnitMeshPool : public UObject
{
    GENERATED_BODY()

public:
    UUnitMeshPool();
    virtual ~UUnitMeshPool();

    /** Initialize the mesh pool */
    void Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem, int32 PoolSize);
    
    /** Deinitialize the mesh pool */
    void Deinitialize();
    
    /** Get a mesh for a unit */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    USkeletalMeshComponent* GetMeshForUnit(FMassEntityHandle Entity);
    
    /** Release a mesh back to the pool */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    void ReleaseMesh(USkeletalMeshComponent* Mesh);
    
    /** Transition a unit to skeletal mesh representation */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool TransitionToSkeletal(FMassEntityHandle Entity);
    
    /** Transition a unit to vertex animation representation */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool TransitionToVertex(FMassEntityHandle Entity);

private:
    /** Reference to the world */
    UPROPERTY(Transient)
    UWorld* World;
    
    /** Reference to the Mass Entity Subsystem */
    UPROPERTY(Transient)
    UMassEntitySubsystem* EntitySubsystem;
    
    /** Pool of available skeletal mesh components */
    UPROPERTY(Transient)
    TArray<USkeletalMeshComponent*> AvailableMeshes;
    
    /** Map of entity handles to skeletal mesh components */
    TMap<FMassEntityHandle, USkeletalMeshComponent*> EntityMeshMap;
    
    /** Map of skeletal mesh components to entity handles */
    TMap<USkeletalMeshComponent*, FMassEntityHandle> MeshEntityMap;
    
    /** Maximum size of the pool */
    int32 MaxPoolSize;
    
    /** Create a new skeletal mesh component */
    USkeletalMeshComponent* CreateMeshComponent();
    
    /** Update mesh from entity data */
    void UpdateMeshFromEntity(USkeletalMeshComponent* Mesh, FMassEntityHandle Entity);
};
