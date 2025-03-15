// Copyright Your Company. All Rights Reserved.

#include "Navigation/FormationSystem.h"
#include "MassEntitySubsystem.h"
#include "Entity/MassUnitFragments.h"
#include "MassUnitCommonFragments.h"
#include "MassEntityView.h"
#include "Engine/World.h"

UFormationSystem::UFormationSystem()
    : EntitySubsystem(nullptr)
    , NextFormationHandle(1)
{
}

UFormationSystem::~UFormationSystem()
{
}

void UFormationSystem::Initialize(UMassEntitySubsystem* InEntitySubsystem)
{
    EntitySubsystem = InEntitySubsystem;
    
    UE_LOG(LogTemp, Log, TEXT("FormationSystem: Initialized"));
}

void UFormationSystem::Deinitialize()
{
    // Clear formations
    Formations.Empty();
    EntityFormationMap.Empty();
    
    // Clear references
    EntitySubsystem = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("FormationSystem: Deinitialized"));
}

void UFormationSystem::Tick(float DeltaTime)
{
    // Update formation movement
    UpdateFormationMovement(DeltaTime);
    
    // Update formation positions
    UpdateFormationPositions(DeltaTime);
}

int32 UFormationSystem::CreateFormation(FVector Location, FRotator Rotation, FName FormationType)
{
    // Create formation data
    FFormationData FormationData;
    FormationData.Location = Location;
    FormationData.Rotation = Rotation;
    FormationData.TargetLocation = Location;
    FormationData.FormationType = FormationType;
    
    // Set default formation shape based on type
    if (FormationType == "Infantry")
    {
        FormationData.FormationShape = "Rectangle";
        FormationData.FormationWidth = 1000.0f;
        FormationData.FormationDepth = 500.0f;
        FormationData.UnitSpacing = 150.0f;
    }
    else if (FormationType == "Cavalry")
    {
        FormationData.FormationShape = "Wedge";
        FormationData.FormationWidth = 800.0f;
        FormationData.FormationDepth = 800.0f;
        FormationData.UnitSpacing = 200.0f;
    }
    else if (FormationType == "Archers")
    {
        FormationData.FormationShape = "Line";
        FormationData.FormationWidth = 1200.0f;
        FormationData.FormationDepth = 300.0f;
        FormationData.UnitSpacing = 180.0f;
    }
    else
    {
        // Default to rectangle
        FormationData.FormationShape = "Rectangle";
    }
    
    // Get formation handle
    int32 FormationHandle = NextFormationHandle++;
    
    // Add to map
    Formations.Add(FormationHandle, FormationData);
    
    UE_LOG(LogTemp, Log, TEXT("FormationSystem: Created formation %d of type %s at %s"), 
        FormationHandle, *FormationType.ToString(), *Location.ToString());
    
    return FormationHandle;
}

bool UFormationSystem::AddEntityToFormation(FMassEntityHandle Entity, int32 FormationHandle)
{
    // Skip if not initialized
    if (!EntitySubsystem)
    {
        return false;
    }
    
    // Get entity manager
    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    
    // Skip if entity is invalid
    if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
    {
        return false;
    }
    
    // Get formation data
    FFormationData* FormationData = Formations.Find(FormationHandle);
    if (!FormationData)
    {
        UE_LOG(LogTemp, Warning, TEXT("FormationSystem: Formation %d not found"), FormationHandle);
        return false;
    }
    
    // Check if entity is already in a formation
    int32* ExistingFormationHandle = EntityFormationMap.Find(Entity);
    if (ExistingFormationHandle)
    {
        // Remove from existing formation
        RemoveEntityFromFormation(Entity);
    }
    
    // Assign slot to entity
    int32 SlotIndex = AssignSlotToEntity(*FormationData, Entity);
    
    // Add entity to formation
    FormationData->Entities.Add(Entity);
    FormationData->EntitySlots.Add(Entity, SlotIndex);
    
    // Add to entity formation map
    EntityFormationMap.Add(Entity, FormationHandle);
    
    // Update entity formation data
    UpdateEntityFormationData(Entity, FormationHandle, SlotIndex);
    
    UE_LOG(LogTemp, Verbose, TEXT("FormationSystem: Added entity %s to formation %d in slot %d"), 
        *Entity.ToString(), FormationHandle, SlotIndex);
    
    return true;
}

bool UFormationSystem::RemoveEntityFromFormation(FMassEntityHandle Entity)
{
    // Skip if not initialized
    if (!EntitySubsystem)
    {
        return false;
    }
    
    // Get entity manager
    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    
    // Skip if entity is invalid
    if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
    {
        return false;
    }
    
    // Get formation handle
    int32* FormationHandlePtr = EntityFormationMap.Find(Entity);
    if (!FormationHandlePtr)
    {
        // Entity not in a formation
        return false;
    }
    
    // Get formation handle
    int32 FormationHandle = *FormationHandlePtr;
    
    // Get formation data
    FFormationData* FormationData = Formations.Find(FormationHandle);
    if (!FormationData)
    {
        // Formation not found, just remove from map
        EntityFormationMap.Remove(Entity);
        return false;
    }
    
    // Remove entity from formation
    FormationData->Entities.Remove(Entity);
    FormationData->EntitySlots.Remove(Entity);
    
    // Remove from entity formation map
    EntityFormationMap.Remove(Entity);
    
    // Update entity formation data
    FMassEntityView EntityView(EntityManager, Entity);
    if (EntityView.HasFragmentData<FMassUnitFormationFragment>())
    {
        FMassUnitFormationFragment& FormationFragment = EntityView.GetFragmentData<FMassUnitFormationFragment>();
        FormationFragment.FormationHandle = 0;
        FormationFragment.FormationSlot = -1;
        FormationFragment.FormationOffset = FVector::ZeroVector;
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("FormationSystem: Removed entity %s from formation %d"), 
        *Entity.ToString(), FormationHandle);
    
    return true;
}

bool UFormationSystem::SetFormationTarget(int32 FormationHandle, FVector TargetLocation)
{
    // Get formation data
    FFormationData* FormationData = Formations.Find(FormationHandle);
    if (!FormationData)
    {
        UE_LOG(LogTemp, Warning, TEXT("FormationSystem: Formation %d not found"), FormationHandle);
        return false;
    }
    
    // Set target location
    FormationData->TargetLocation = TargetLocation;
    FormationData->bIsMoving = true;
    
    UE_LOG(LogTemp, Verbose, TEXT("FormationSystem: Set formation %d target to %s"), 
        FormationHandle, *TargetLocation.ToString());
    
    return true;
}

bool UFormationSystem::SetFormationShape(int32 FormationHandle, FName FormationShape)
{
    // Get formation data
    FFormationData* FormationData = Formations.Find(FormationHandle);
    if (!FormationData)
    {
        UE_LOG(LogTemp, Warning, TEXT("FormationSystem: Formation %d not found"), FormationHandle);
        return false;
    }
    
    // Set formation shape
    FormationData->FormationShape = FormationShape;
    
    // Update all entities in formation
    for (int32 i = 0; i < FormationData->Entities.Num(); ++i)
    {
        FMassEntityHandle Entity = FormationData->Entities[i];
        int32 SlotIndex = FormationData->EntitySlots.FindChecked(Entity);
        
        // Update entity formation data
        UpdateEntityFormationData(Entity, FormationHandle, SlotIndex);
    }
    
    UE_LOG(LogTemp, Log, TEXT("FormationSystem: Set formation %d shape to %s"), 
        FormationHandle, *FormationShape.ToString());
    
    return true;
}

FVector UFormationSystem::GetFormationLocation(int32 FormationHandle) const
{
    // Get formation data
    const FFormationData* FormationData = Formations.Find(FormationHandle);
    if (!FormationData)
    {
        return FVector::ZeroVector;
    }
    
    return FormationData->Location;
}

FRotator UFormationSystem::GetFormationRotation(int32 FormationHandle) const
{
    // Get formation data
    const FFormationData* FormationData = Formations.Find(FormationHandle);
    if (!FormationData)
    {
        return FRotator::ZeroRotator;
    }
    
    return FormationData->Rotation;
}

TArray<FMassEntityHandle> UFormationSystem::GetEntitiesInFormation(int32 FormationHandle) const
{
    // Get formation data
    const FFormationData* FormationData = Formations.Find(FormationHandle);
    if (!FormationData)
    {
        return TArray<FMassEntityHandle>();
    }
    
    return FormationData->Entities;
}

void UFormationSystem::UpdateFormationPositions(float DeltaTime)
{
    // Skip if not initialized
    if (!EntitySubsystem)
    {
        return;
    }
    
    // Get entity manager
    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    
    // Update each formation
    for (auto& Pair : Formations)
    {
        int32 FormationHandle = Pair.Key;
        FFormationData& FormationData = Pair.Value;
        
        // Skip if no entities
        if (FormationData.Entities.Num() == 0)
        {
            continue;
        }
        
        // Update each entity in formation
        for (int32 i = 0; i < FormationData.Entities.Num(); ++i)
        {
            FMassEntityHandle Entity = FormationData.Entities[i];
            
            // Skip if entity is invalid
            if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
            {
                continue;
            }
            
            // Get entity view
            FMassEntityView EntityView(EntityManager, Entity);
            
            // Skip if missing required fragments
            if (!EntityView.HasFragmentData<FMassUnitFormationFragment>() ||
                !EntityView.HasFragmentData<FMassUnitTargetFragment>())
            {
                continue;
            }
            
            // Get fragments
            FMassUnitFormationFragment& FormationFragment = EntityView.GetFragmentData<FMassUnitFormationFragment>();
            FMassUnitTargetFragment& TargetFragment = EntityView.GetFragmentData<FMassUnitTargetFragment>();
            
            // Get slot index
            int32 SlotIndex = FormationData.EntitySlots.FindChecked(Entity);
            
            // Calculate slot position
            FVector SlotOffset = CalculateSlotPosition(FormationData, SlotIndex);
            
            // Update formation fragment
            FormationFragment.FormationHandle = FormationHandle;
            FormationFragment.FormationSlot = SlotIndex;
            FormationFragment.FormationOffset = SlotOffset;
            
            // Update target fragment
            TargetFragment.TargetLocation = FormationData.Location + FormationData.Rotation.RotateVector(SlotOffset);
        }
    }
}

void UFormationSystem::UpdateFormationMovement(float DeltaTime)
{
    // Update each formation
    for (auto& Pair : Formations)
    {
        int32 FormationHandle = Pair.Key;
        FFormationData& FormationData = Pair.Value;
        
        // Skip if not moving
        if (!FormationData.bIsMoving)
        {
            continue;
        }
        
        // Calculate direction to target
        FVector Direction = FormationData.TargetLocation - FormationData.Location;
        float DistanceSquared = Direction.SizeSquared();
        
        // Check if reached target
        if (DistanceSquared <= 100.0f)
        {
            // Reached target
            FormationData.Location = FormationData.TargetLocation;
            FormationData.bIsMoving = false;
            
            UE_LOG(LogTemp, Verbose, TEXT("FormationSystem: Formation %d reached target %s"), 
                FormationHandle, *FormationData.TargetLocation.ToString());
            
            continue;
        }
        
        // Normalize direction
        Direction.Normalize();
        
        // Calculate movement speed
        float Speed = 300.0f; // Units per second
        
        // Calculate movement delta
        FVector MovementDelta = Direction * Speed * DeltaTime;
        
        // Check if movement would overshoot target
        if (MovementDelta.SizeSquared() >= DistanceSquared)
        {
            // Reached target
            FormationData.Location = FormationData.TargetLocation;
            FormationData.bIsMoving = false;
        }
        else
        {
            // Move towards target
            FormationData.Location += MovementDelta;
        }
        
        // Update rotation to face movement direction
        FormationData.Rotation = Direction.Rotation();
    }
}

FVector UFormationSystem::CalculateSlotPosition(const FFormationData& Formation, int32 SlotIndex)
{
    // Calculate slot position based on formation shape
    if (Formation.FormationShape == "Rectangle")
    {
        // Calculate rows and columns
        int32 UnitsPerRow = FMath::Max(1, FMath::FloorToInt(Formation.FormationWidth / Formation.UnitSpacing));
        int32 Row = SlotIndex / UnitsPerRow;
        int32 Column = SlotIndex % UnitsPerRow;
        
        // Calculate offset
        float OffsetX = (Column - UnitsPerRow / 2.0f) * Formation.UnitSpacing;
        float OffsetY = Row * Formation.UnitSpacing;
        
        return FVector(OffsetY, OffsetX, 0.0f);
    }
    else if (Formation.FormationShape == "Wedge")
    {
        // Calculate rows
        int32 Row = FMath::Sqrt(2.0f * SlotIndex);
        int32 RowStartIndex = (Row * (Row + 1)) / 2;
        int32 Column = SlotIndex - RowStartIndex;
        
        // Calculate offset
        float OffsetX = (Column - Row / 2.0f) * Formation.UnitSpacing;
        float OffsetY = Row * Formation.UnitSpacing;
        
        return FVector(OffsetY, OffsetX, 0.0f);
    }
    else if (Formation.FormationShape == "Line")
    {
        // Calculate offset
        float OffsetX = (SlotIndex - Formation.Entities.Num() / 2.0f) * Formation.UnitSpacing;
        
        return FVector(0.0f, OffsetX, 0.0f);
    }
    else if (Formation.FormationShape == "Circle")
    {
        // Calculate angle
        float Angle = (SlotIndex * 2.0f * PI) / FMath::Max(1, Formation.Entities.Num());
        
        // Calculate radius
        float Radius = FMath::Sqrt(Formation.Entities.Num() * Formation.UnitSpacing * Formation.UnitSpacing / PI);
        
        // Calculate offset
        float OffsetX = FMath::Cos(Angle) * Radius;
        float OffsetY = FMath::Sin(Angle) * Radius;
        
        return FVector(OffsetY, OffsetX, 0.0f);
    }
    
    // Default to grid
    int32 GridSize = FMath::CeilToInt(FMath::Sqrt(Formation.Entities.Num()));
    int32 Row = SlotIndex / GridSize;
    int32 Column = SlotIndex % GridSize;
    
    float OffsetX = (Column - GridSize / 2.0f) * Formation.UnitSpacing;
    float OffsetY = (Row - GridSize / 2.0f) * Formation.UnitSpacing;
    
    return FVector(OffsetY, OffsetX, 0.0f);
}

int32 UFormationSystem::AssignSlotToEntity(FFormationData& Formation, FMassEntityHandle Entity)
{
    // Check if entity already has a slot
    int32* ExistingSlot = Formation.EntitySlots.Find(Entity);
    if (ExistingSlot)
    {
        return *ExistingSlot;
    }
    
    // Find next available slot
    int32 SlotIndex = Formation.Entities.Num();
    
    return SlotIndex;
}

void UFormationSystem::UpdateEntityFormationData(FMassEntityHandle Entity, int32 FormationHandle, int32 SlotIndex)
{
    // Skip if not initialized
    if (!EntitySubsystem)
    {
        return;
    }
    
    // Get entity manager
    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    
    // Skip if entity is invalid
    if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
    {
        return;
    }
    
    // Get entity view
    FMassEntityView EntityView(EntityManager, Entity);
    
    // Skip if missing required fragments
    if (!EntityView.HasFragmentData<FMassUnitFormationFragment>())
    {
        return;
    }
    
    // Get formation data
    FFormationData* FormationData = Formations.Find(FormationHandle);
    if (!FormationData)
    {
        return;
    }
    
    // Get formation fragment
    FMassUnitFormationFragment& FormationFragment = EntityView.GetFragmentData<FMassUnitFormationFragment>();
    
    // Update formation fragment
    FormationFragment.FormationHandle = FormationHandle;
    FormationFragment.FormationSlot = SlotIndex;
    
    // Calculate slot position
    FVector SlotOffset = CalculateSlotPosition(*FormationData, SlotIndex);
    FormationFragment.FormationOffset = SlotOffset;
    
    // Update target fragment if available
    if (EntityView.HasFragmentData<FMassUnitTargetFragment>())
    {
        FMassUnitTargetFragment& TargetFragment = EntityView.GetFragmentData<FMassUnitTargetFragment>();
        TargetFragment.TargetLocation = FormationData->Location + FormationData->Rotation.RotateVector(SlotOffset);
    }
}
