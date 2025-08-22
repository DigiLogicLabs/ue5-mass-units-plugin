// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Navigation/MassUnitNavigationSystem.h"
#include "MassEntitySubsystem.h"
#include "Entity/MassUnitFragments.h"
#include "MassUnitCommonFragments.h"
#include "MassEntityView.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "NavMesh/RecastNavMesh.h"
#include "Engine/World.h"

UMassUnitNavigationSystem::UMassUnitNavigationSystem()
    : World(nullptr)
    , EntitySubsystem(nullptr)
    , NavigationSystem(nullptr)
    , NavigationData(nullptr)
{
}

UMassUnitNavigationSystem::~UMassUnitNavigationSystem()
{
}

void UMassUnitNavigationSystem::Initialize(UWorld* InWorld, UMassUnitEntitySubsystem* InEntitySubsystem)
{
    World = InWorld;
    EntitySubsystem = InEntitySubsystem;
    
    // Get navigation system
    if (World)
    {
        NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (NavigationSystem)
        {
            // Get navigation data
            NavigationData = NavigationSystem->GetDefaultNavDataInstance();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitNavigationSystem: Initialized with nav system %p, nav data %p"), 
        NavigationSystem, NavigationData);
}

void UMassUnitNavigationSystem::Deinitialize()
{
    // Clear path requests
    PathRequestQueue.Empty();
    EntityPathMap.Empty();
    
    // Clear references
    NavigationData = nullptr;
    NavigationSystem = nullptr;
    EntitySubsystem = nullptr;
    World = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitNavigationSystem: Deinitialized"));
}

void UMassUnitNavigationSystem::UpdateNavigationData(UWorld* InWorld)
{
    if (!InWorld)
    {
        return;
    }
    NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InWorld);
    if (NavigationSystem)
    {
        NavigationData = NavigationSystem->GetDefaultNavDataInstance();
    }
    UE_LOG(LogTemp, Log, TEXT("MassUnitNavigationSystem: Updated navigation data"));
}

bool UMassUnitNavigationSystem::RequestPathInternal(FMassUnitEntityHandle Entity, const FVector& Destination)
{
    // Skip if not initialized
    if (!EntitySubsystem || !NavigationSystem || !NavigationData)
    {
        return false;
    }
    
    // Get entity manager
    FMassUnitEntityManagerFallback& EntityManager = *EntitySubsystem->GetMutableUnitEntityManager();
    
    // Skip if entity is invalid
    if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
    {
        return false;
    }
    
    // Get entity view
    FMassUnitEntityView EntityView(EntityManager, Entity);
    
    // Skip if missing required fragments
    if (!EntityView.HasFragmentData<FMassUnitTransformFragment>() ||
        !EntityView.HasFragmentData<FMassUnitNavigationFragment>())
    {
        return false;
    }
    
    // Get fragments
    const FMassUnitTransformFragment& TransformFragment = EntityView.GetFragmentData<FMassUnitTransformFragment>();
    FMassUnitNavigationFragment& NavigationFragment = EntityView.GetFragmentData<FMassUnitNavigationFragment>();
    
    // Get current location
    FVector CurrentLocation = TransformFragment.GetTransform().GetLocation();
    
    // Create path request
    FPathRequest Request;
    Request.Entity = Entity;
    Request.Destination = Destination;
    Request.ResultPath = MakeShared<FNavPathSharedPtr>();
    
    // Set up delegate
    Request.Delegate.BindUObject(this, &UMassUnitNavigationSystem::HandlePathRequestComplete);
    
    // Add to queue
    PathRequestQueue.Add(Request);
    
    // Update navigation fragment
    NavigationFragment.DestinationLocation = Destination;
    NavigationFragment.bPathRequested = true;
    NavigationFragment.bPathValid = false;
    
    UE_LOG(LogTemp, Verbose, TEXT("MassUnitNavigationSystem: Requested path for entity %s from %s to %s"), 
        *Entity.ToString(), *CurrentLocation.ToString(), *Destination.ToString());
    
    return true;
}

void UMassUnitNavigationSystem::ProcessPathRequests()
{
    // Skip if not initialized
    if (!NavigationSystem || !NavigationData || PathRequestQueue.Num() == 0)
    {
        return;
    }
    
    // Process a batch of path requests
    ProcessPathRequestBatch(FMath::Min(PathRequestQueue.Num(), MaxPathRequestsPerFrame));
}

void UMassUnitNavigationSystem::ProcessPathRequestBatch(int32 BatchSize)
{
    // Skip if no requests
    if (PathRequestQueue.Num() == 0 || BatchSize <= 0)
    {
        return;
    }
    
    // Process requests in batches
    int32 BatchesProcessed = 0;
    int32 RequestsProcessed = 0;
    
    while (PathRequestQueue.Num() > 0 && RequestsProcessed < BatchSize)
    {
        // Get batch size
        int32 CurrentBatchSize = FMath::Min(PathRequestQueue.Num(), PathRequestBatchSize);
        
        // Process batch
        for (int32 i = 0; i < CurrentBatchSize; ++i)
        {
            // Get request
            FPathRequest& Request = PathRequestQueue[0];
            
            // Get entity manager
            FMassUnitEntityManagerFallback& EntityManager = *EntitySubsystem->GetMutableUnitEntityManager();
            
            // Skip if entity is invalid
            if (!Request.Entity.IsValid() || !EntityManager.IsEntityValid(Request.Entity))
            {
                PathRequestQueue.RemoveAt(0);
                continue;
            }
            
            // Get entity view
            FMassUnitEntityView EntityView(EntityManager, Request.Entity);
            
            // Skip if missing required fragments
            if (!EntityView.HasFragmentData<FMassUnitTransformFragment>())
            {
                PathRequestQueue.RemoveAt(0);
                continue;
            }
            
            // Get fragments
            const FMassUnitTransformFragment& TransformFragment = EntityView.GetFragmentData<FMassUnitTransformFragment>();
            
            // Get current location
            FVector CurrentLocation = TransformFragment.GetTransform().GetLocation();
            
            // Create path finding query
            FPathFindingQuery Query(nullptr, *NavigationData, CurrentLocation, Request.Destination);
            
            // Find path async
            NavigationSystem->FindPathAsync(FNavAgentProperties::DefaultProperties, Query, Request.Delegate);
            
            // Remove request from queue
            PathRequestQueue.RemoveAt(0);
            
            // Increment counter
            RequestsProcessed++;
        }
        
        // Increment batch counter
        BatchesProcessed++;
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("MassUnitNavigationSystem: Processed %d path requests in %d batches"), 
        RequestsProcessed, BatchesProcessed);
}

void UMassUnitNavigationSystem::HandlePathRequestComplete(uint32 PathId, ENavigationQueryResult::Type Result, FNavPathSharedPtr Path)
{
    // Skip if path is invalid
    if (Result != ENavigationQueryResult::Success || !Path.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MassUnitNavigationSystem: Path request failed with result %d"), Result);
        return;
    }
    
    // Get path owner
    FMassUnitEntityHandle Entity = FMassUnitEntityHandle();
    
    // Find entity for this path
    for (auto& Pair : EntityPathMap)
    {
        if (Pair.Value.Get() == Path.Get())
        {
            Entity = Pair.Key;
            break;
        }
    }
    
    // Skip if no entity found
    if (!Entity.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MassUnitNavigationSystem: Path owner not found for path %d"), PathId);
        return;
    }
    
    // Update entity with path
    UpdateEntityWithPath(Entity, Path);
}

void UMassUnitNavigationSystem::UpdateEntityWithPath(FMassUnitEntityHandle Entity, const FNavPathSharedPtr& Path)
{
    // Skip if not initialized
    if (!EntitySubsystem)
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
    if (!EntityView.HasFragmentData<FMassUnitNavigationFragment>())
    {
        return;
    }
    
    // Get navigation fragment
    FMassUnitNavigationFragment& NavigationFragment = EntityView.GetFragmentData<FMassUnitNavigationFragment>();
    
    // Update path
    EntityPathMap.Add(Entity, Path);
    
    // Update navigation fragment
    NavigationFragment.bPathRequested = false;
    NavigationFragment.bPathValid = true;
    NavigationFragment.PathPoints.Empty();
    
    // Copy path points
    if (Path.IsValid() && Path->GetPathPoints().Num() > 0)
    {
        for (const FNavPathPoint& Point : Path->GetPathPoints())
        {
            NavigationFragment.PathPoints.Add(Point.Location);
        }
        
        // Set current path index
        NavigationFragment.CurrentPathIndex = 0;
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("MassUnitNavigationSystem: Updated entity %s with path (%d points)"), 
        *Entity.ToString(), NavigationFragment.PathPoints.Num());
}
