// Copyright Your Company. All Rights Reserved.

#include "Visual/NiagaraUnitSystem.h"
#include "MassEntitySubsystem.h"
#include "Visual/VertexAnimationManager.h"
#include "Entity/MassUnitFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"

UNiagaraUnitSystem::UNiagaraUnitSystem()
    : World(nullptr)
    , EntitySubsystem(nullptr)
    , NiagaraSystemAsset(nullptr)
    , NiagaraComponent(nullptr)
    , VertexAnimationManager(nullptr)
    , CurrentLODLevel(0)
    , LastUpdateTime(0.0f)
{
}

UNiagaraUnitSystem::~UNiagaraUnitSystem()
{
}

void UNiagaraUnitSystem::Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem)
{
    World = InWorld;
    EntitySubsystem = InEntitySubsystem;
    
    // Create Niagara system
    CreateNiagaraSystem();
    
    // Create vertex animation manager
    VertexAnimationManager = NewObject<UVertexAnimationManager>(this);
    if (VertexAnimationManager)
    {
        VertexAnimationManager->Initialize();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("NiagaraUnitSystem: Failed to create VertexAnimationManager"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("NiagaraUnitSystem: Initialized"));
}

void UNiagaraUnitSystem::Deinitialize()
{
    // Clean up Niagara component
    if (NiagaraComponent)
    {
        NiagaraComponent->DestroyComponent();
        NiagaraComponent = nullptr;
    }
    
    // Clean up vertex animation manager
    if (VertexAnimationManager)
    {
        VertexAnimationManager->Deinitialize();
        VertexAnimationManager = nullptr;
    }
    
    // Clean up references
    NiagaraSystemAsset = nullptr;
    EntitySubsystem = nullptr;
    World = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("NiagaraUnitSystem: Deinitialized"));
}

void UNiagaraUnitSystem::UpdateUnitVisuals(const TArray<FMassEntityHandle>& Entities)
{
    // Skip if not initialized
    if (!World || !EntitySubsystem || !NiagaraComponent)
    {
        return;
    }
    
    // Check if it's time to update
    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < UpdateFrequency)
    {
        return;
    }
    
    // Update unit data
    UpdateUnitData(Entities);
    
    // Update last update time
    LastUpdateTime = CurrentTime;
}

void UNiagaraUnitSystem::SetLODLevel(int32 LODLevel)
{
    // Skip if not initialized
    if (!NiagaraComponent)
    {
        return;
    }
    
    // Skip if LOD level hasn't changed
    if (CurrentLODLevel == LODLevel)
    {
        return;
    }
    
    // Update LOD level
    CurrentLODLevel = LODLevel;
    
    // Set LOD level parameter in Niagara
    NiagaraComponent->SetIntParameter(FName("LODLevel"), CurrentLODLevel);
    
    UE_LOG(LogTemp, Log, TEXT("NiagaraUnitSystem: Set LOD level to %d"), CurrentLODLevel);
}

void UNiagaraUnitSystem::CreateNiagaraSystem()
{
    // Skip if already created
    if (NiagaraComponent)
    {
        return;
    }
    
    // Skip if no world
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("NiagaraUnitSystem: Failed to create Niagara system - no world"));
        return;
    }
    
    // Load Niagara system asset
    // In a real implementation, this would load from a path or be set in the editor
    // For this example, we'll assume it's already loaded
    // NiagaraSystemAsset = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/MassUnitSystem/NS_UnitSystem"));
    
    // For now, create a dummy Niagara system
    NiagaraSystemAsset = NewObject<UNiagaraSystem>(this);
    
    // Skip if failed to load
    if (!NiagaraSystemAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("NiagaraUnitSystem: Failed to load Niagara system asset"));
        return;
    }
    
    // Create Niagara component
    NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        NiagaraSystemAsset,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::AutoRelease,
        true
    );
    
    // Skip if failed to create
    if (!NiagaraComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("NiagaraUnitSystem: Failed to create Niagara component"));
        return;
    }
    
    // Set initial parameters
    NiagaraComponent->SetIntParameter(FName("MaxUnits"), MaxUnits);
    NiagaraComponent->SetIntParameter(FName("LODLevel"), CurrentLODLevel);
    
    UE_LOG(LogTemp, Log, TEXT("NiagaraUnitSystem: Created Niagara system"));
}

void UNiagaraUnitSystem::UpdateUnitData(const TArray<FMassEntityHandle>& Entities)
{
    // Skip if not initialized
    if (!EntitySubsystem || !NiagaraComponent)
    {
        return;
    }
    
    // Get entity manager
    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    
    // Prepare arrays for Niagara
    TArray<FVector> Positions;
    TArray<FVector> Velocities;
    TArray<FVector> Scales;
    TArray<FQuat> Rotations;
    TArray<int32> TeamIDs;
    TArray<FLinearColor> TeamColors;
    TArray<int32> AnimationIndices;
    TArray<float> AnimationTimes;
    TArray<int32> LODLevels;
    TArray<int32> VisibilityFlags;
    
    // Reserve space
    int32 NumEntities = FMath::Min(Entities.Num(), MaxUnits);
    Positions.Reserve(NumEntities);
    Velocities.Reserve(NumEntities);
    Scales.Reserve(NumEntities);
    Rotations.Reserve(NumEntities);
    TeamIDs.Reserve(NumEntities);
    TeamColors.Reserve(NumEntities);
    AnimationIndices.Reserve(NumEntities);
    AnimationTimes.Reserve(NumEntities);
    LODLevels.Reserve(NumEntities);
    VisibilityFlags.Reserve(NumEntities);
    
    // Process entities
    for (int32 i = 0; i < NumEntities; ++i)
    {
        FMassEntityHandle Entity = Entities[i];
        
        // Skip invalid entities
        if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
        {
            continue;
        }
        
        // Get entity view
        FMassEntityView EntityView(EntityManager, Entity);
        
        // Skip if missing required fragments
        if (!EntityView.HasFragmentData<FTransformFragment>() ||
            !EntityView.HasFragmentData<FMassVelocityFragment>() ||
            !EntityView.HasFragmentData<FMassUnitVisualFragment>() ||
            !EntityView.HasFragmentData<FMassUnitTeamFragment>() ||
            !EntityView.HasFragmentData<FMassUnitStateFragment>())
        {
            continue;
        }
        
        // Get fragments
        const FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
        const FMassVelocityFragment& VelocityFragment = EntityView.GetFragmentData<FMassVelocityFragment>();
        const FMassUnitVisualFragment& VisualFragment = EntityView.GetFragmentData<FMassUnitVisualFragment>();
        const FMassUnitTeamFragment& TeamFragment = EntityView.GetFragmentData<FMassUnitTeamFragment>();
        const FMassUnitStateFragment& StateFragment = EntityView.GetFragmentData<FMassUnitStateFragment>();
        
        // Skip if using skeletal mesh
        if (VisualFragment.bUseSkeletalMesh)
        {
            continue;
        }
        
        // Skip if not visible
        if (!VisualFragment.bIsVisible)
        {
            continue;
        }
        
        // Get transform
        FTransform Transform = TransformFragment.GetTransform();
        
        // Add data to arrays
        Positions.Add(Transform.GetLocation());
        Velocities.Add(VelocityFragment.Value);
        Scales.Add(Transform.GetScale3D());
        Rotations.Add(Transform.GetRotation());
        TeamIDs.Add(TeamFragment.TeamID);
        TeamColors.Add(TeamFragment.TeamColor);
        
        // Convert animation tag to index
        // In a real implementation, this would map from animation tag to index
        int32 AnimationIndex = static_cast<int32>(StateFragment.CurrentState);
        AnimationIndices.Add(AnimationIndex);
        
        // Add animation time
        AnimationTimes.Add(StateFragment.StateTime);
        
        // Add LOD level
        LODLevels.Add(VisualFragment.LODLevel);
        
        // Add visibility flag
        VisibilityFlags.Add(VisualFragment.bIsVisible ? 1 : 0);
    }
    
    // Set arrays in Niagara
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, FName("UnitPositions"), Positions);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, FName("UnitVelocities"), Velocities);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, FName("UnitScales"), Scales);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(NiagaraComponent, FName("UnitRotations"), Rotations);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayInt(NiagaraComponent, FName("UnitTeamIDs"), TeamIDs);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayLinearColor(NiagaraComponent, FName("UnitTeamColors"), TeamColors);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayInt(NiagaraComponent, FName("UnitAnimationIndices"), AnimationIndices);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComponent, FName("UnitAnimationTimes"), AnimationTimes);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayInt(NiagaraComponent, FName("UnitLODLevels"), LODLevels);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayInt(NiagaraComponent, FName("UnitVisibilityFlags"), VisibilityFlags);
    
    // Set unit count
    NiagaraComponent->SetIntParameter(FName("UnitCount"), Positions.Num());
    
    UE_LOG(LogTemp, Verbose, TEXT("NiagaraUnitSystem: Updated %d units"), Positions.Num());
}
