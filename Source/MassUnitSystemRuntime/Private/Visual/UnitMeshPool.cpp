// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Visual/UnitMeshPool.h"
#include "MassEntityTypes.h"
#include "Entity/MassUnitEntityManager.h"
#include "Entity/MassEntityFallback.h"
#include "Entity/MassUnitFragments.h"
#include "Entity/UnitTemplate.h"
#include "MassUnitCommonFragments.h"
#include "MassEntityView.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"

// ...existing code...
// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Visual/UnitMeshPool.h"
#include "MassEntitySubsystem.h"
#include "Entity/MassUnitFragments.h"
#include "Entity/UnitTemplate.h"
#include "MassUnitCommonFragments.h"
#include "MassEntityView.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"

UUnitMeshPool::UUnitMeshPool()
    : World(nullptr)
    , EntitySubsystem(nullptr)
    , MaxPoolSize(100)
{
}

UUnitMeshPool::~UUnitMeshPool()
{
}

void UUnitMeshPool::Initialize(UWorld* InWorld, UMassUnitEntitySubsystem* InEntitySubsystem, int32 PoolSize)
{
    World = InWorld;
    EntitySubsystem = InEntitySubsystem;
    MaxPoolSize = FMath::Max(1, PoolSize);
    
    // Pre-create some mesh components
    int32 PreCreateCount = FMath::Min(MaxPoolSize / 4, 25); // Pre-create 25% of pool size, max 25
    for (int32 i = 0; i < PreCreateCount; ++i)
    {
        USkeletalMeshComponent* MeshComponent = this->CreateMeshComponent();
        if (MeshComponent)
        {
            AvailableMeshes.Add(MeshComponent);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("UnitMeshPool: Initialized with pool size %d, pre-created %d meshes"), 
        MaxPoolSize, AvailableMeshes.Num());
}

void UUnitMeshPool::Deinitialize()
{
    // Release all meshes
    for (auto& Pair : EntityMeshMap)
    {
        USkeletalMeshComponent* Mesh = Pair.Value;
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    
    // Clear maps
    EntityMeshMap.Empty();
    MeshEntityMap.Empty();
    
    // Destroy available meshes
    for (USkeletalMeshComponent* Mesh : AvailableMeshes)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    AvailableMeshes.Empty();
    
    // Clear references
    EntitySubsystem = nullptr;
    World = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("UnitMeshPool: Deinitialized"));
}

USkeletalMeshComponent* UUnitMeshPool::GetMeshForUnitInternal(FMassUnitEntityHandle Entity)
{
    // Check if entity already has a mesh
    if (USkeletalMeshComponent** MeshPtr = EntityMeshMap.Find(Entity))
    {
        return *MeshPtr;
    }
    
    // Check if we've reached the pool size limit
    if (EntityMeshMap.Num() >= MaxPoolSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnitMeshPool: Pool size limit reached, cannot allocate more meshes"));
        return nullptr;
    }
    
    // Get a mesh from the pool or create a new one
    USkeletalMeshComponent* Mesh = nullptr;
    if (AvailableMeshes.Num() > 0)
    {
        Mesh = AvailableMeshes.Pop();
    }
    else
    {
        Mesh = this->CreateMeshComponent();
    }
    
    // Skip if failed to get a mesh
    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("UnitMeshPool: Failed to get mesh for entity %s"), *Entity.ToString());
        return nullptr;
    }
    
    // Update mesh from entity data
    this->UpdateMeshFromEntity(Mesh, Entity);
    
    // Add to maps
    EntityMeshMap.Add(Entity, Mesh);
    MeshEntityMap.Add(Mesh, Entity);
    
    return Mesh;
}

void UUnitMeshPool::ReleaseMesh(USkeletalMeshComponent* Mesh)
{
    // Skip if null
    if (!Mesh)
    {
        return;
    }
    
    // Get entity for this mesh
    FMassUnitEntityHandle* EntityPtr = MeshEntityMap.Find(Mesh);
    if (!EntityPtr)
    {
        // Mesh not in pool, just destroy it
        Mesh->DestroyComponent();
        return;
    }
    
    // Get entity
    FMassUnitEntityHandle Entity = *EntityPtr;
    
    // Remove from maps
    EntityMeshMap.Remove(Entity);
    MeshEntityMap.Remove(Mesh);
    
    // Reset mesh
    Mesh->SetVisibility(false);
    Mesh->SetSkeletalMesh(nullptr);
    
    // Add to available meshes
    AvailableMeshes.Add(Mesh);
}

bool UUnitMeshPool::TransitionToSkeletalInternal(FMassUnitEntityHandle Entity)
{
    // Skip if not initialized
    if (!this->EntitySubsystem || !this->World)
    {
        return false;
    }
    
    // Skip if entity already has a mesh
    if (EntityMeshMap.Contains(Entity))
    {
        return true;
    }
    
    // Get entity manager
    FMassUnitEntityManagerFallback& EntityManager = *this->EntitySubsystem->GetMutableUnitEntityManager();
    
    // Skip if entity is invalid
    if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
    {
        return false;
    }
    
    // Get entity view
    FMassUnitEntityView EntityView(EntityManager, Entity);
    
    // Skip if missing required fragments
    if (!EntityView.HasFragmentData<FMassUnitVisualFragment>())
    {
        return false;
    }
    
    // Get visual fragment
    FMassUnitVisualFragment& VisualFragment = EntityView.GetFragmentData<FMassUnitVisualFragment>();
    
    // Skip if already using skeletal mesh
    if (VisualFragment.bUseSkeletalMesh)
    {
        return true;
    }
    
    // Get a mesh for this entity
    USkeletalMeshComponent* Mesh = this->GetMeshForUnit(Entity);
    if (!Mesh)
    {
        return false;
    }
    
    // Update visual fragment
    VisualFragment.bUseSkeletalMesh = true;
    VisualFragment.SkeletalMeshIndex = EntityMeshMap.Num() - 1; // Index is just for tracking
    
    // Make mesh visible
    Mesh->SetVisibility(true);
    
    UE_LOG(LogTemp, Log, TEXT("UnitMeshPool: Transitioned entity %s to skeletal mesh"), *Entity.ToString());
    
    return true;
}

bool UUnitMeshPool::TransitionToVertexInternal(FMassUnitEntityHandle Entity)
{
    // Skip if not initialized
    if (!this->EntitySubsystem)
    {
        return false;
    }
    
    // Get entity manager
    FMassUnitEntityManagerFallback& EntityManager = *this->EntitySubsystem->GetMutableUnitEntityManager();
    
    // Skip if entity is invalid
    if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
    {
        return false;
    }
    
    // Get entity view
    FMassUnitEntityView EntityView(EntityManager, Entity);
    
    // Skip if missing required fragments
    if (!EntityView.HasFragmentData<FMassUnitVisualFragment>())
    {
        return false;
    }
    
    // Get visual fragment
    FMassUnitVisualFragment& VisualFragment = EntityView.GetFragmentData<FMassUnitVisualFragment>();
    
    // Skip if already using vertex animation
    if (!VisualFragment.bUseSkeletalMesh)
    {
        return true;
    }
    
    // Get mesh for this entity
    USkeletalMeshComponent** MeshPtr = EntityMeshMap.Find(Entity);
    if (!MeshPtr || !*MeshPtr)
    {
        // No mesh found, just update the fragment
        VisualFragment.bUseSkeletalMesh = false;
        VisualFragment.SkeletalMeshIndex = -1;
        return true;
    }
    
    // Get mesh
    USkeletalMeshComponent* Mesh = *MeshPtr;
    
    // Release mesh
    this->ReleaseMesh(Mesh);
    
    // Update visual fragment
    VisualFragment.bUseSkeletalMesh = false;
    VisualFragment.SkeletalMeshIndex = -1;
    
    UE_LOG(LogTemp, Log, TEXT("UnitMeshPool: Transitioned entity %s to vertex animation"), *Entity.ToString());
    
    return true;
}

USkeletalMeshComponent* UUnitMeshPool::CreateMeshComponent()
{
    // Skip if no world
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("UnitMeshPool: Failed to create mesh component - no world"));
        return nullptr;
    }
    
    // Create a new skeletal mesh component
    USkeletalMeshComponent* MeshComponent = NewObject<USkeletalMeshComponent>(this);
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("UnitMeshPool: Failed to create mesh component"));
        return nullptr;
    }
    
    // Register component
    MeshComponent->RegisterComponent();
    
    // Set initial properties
    MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetGenerateOverlapEvents(false);
    
    return MeshComponent;
}

void UUnitMeshPool::UpdateMeshFromEntity(USkeletalMeshComponent* Mesh, FMassUnitEntityHandle Entity)
{
    // Skip if not initialized
    if (!EntitySubsystem || !Mesh)
    {
        return;
    }
    
    // Get entity manager
    FMassUnitEntityManagerFallback& EntityManager = *EntitySubsystem->GetMutableUnitEntityManager();
    
    // Skip if entity is invalid
    if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
    {
        return;
    }
    
    // Get entity view
    FMassUnitEntityView EntityView(EntityManager, Entity);
    
    // Skip if missing required fragments
    if (!EntityView.HasFragmentData<FMassUnitTransformFragment>() || 
        !EntityView.HasFragmentData<FMassUnitVisualFragment>() ||
        !EntityView.HasFragmentData<FMassUnitStateFragment>())
    {
        return;
    }
    
    // Get fragments
    const FMassUnitTransformFragment& TransformFragment = EntityView.GetFragmentData<FMassUnitTransformFragment>();
    const FMassUnitVisualFragment& VisualFragment = EntityView.GetFragmentData<FMassUnitVisualFragment>();
    const FMassUnitStateFragment& StateFragment = EntityView.GetFragmentData<FMassUnitStateFragment>();
    
    // Set transform
    Mesh->SetWorldTransform(TransformFragment.GetTransform());
    
    // In a real implementation, we would load the skeletal mesh from the unit template
    // For this example, we'll just log that we're updating the mesh
    UE_LOG(LogTemp, Verbose, TEXT("UnitMeshPool: Updating mesh for entity %s"), *Entity.ToString());
    
    // For a real implementation, we would do something like:
    /*
    // Get unit template
    UUnitTemplate* Template = ...; // Get from entity or cache
    
    // Set skeletal mesh
    if (Template && !Template->SkeletalMesh.IsNull())
    {
        USkeletalMesh* SkeletalMesh = Template->SkeletalMesh.LoadSynchronous();
        if (SkeletalMesh)
        {
            Mesh->SetSkeletalMesh(SkeletalMesh);
        }
    }
    
    // Set animation
    if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
    {
        // Set animation state
        if (UUnitAnimInstance* UnitAnimInstance = Cast<UUnitAnimInstance>(AnimInstance))
        {
            UnitAnimInstance->SetAnimationState(StateFragment.CurrentState);
            UnitAnimInstance->SetAnimationTime(StateFragment.StateTime);
        }
    }
    */
    
    // Make mesh visible
    Mesh->SetVisibility(true);
}
