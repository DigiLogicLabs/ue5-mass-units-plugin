// Copyright Your Company. All Rights Reserved.

#include "Gameplay/MassUnitBehaviorIntegration.h"
#include "Gameplay/GASUnitIntegration.h"
#include "Entity/MassUnitFragments.h"

// Include MassEntity types or fallback
#if WITH_MASSENTITY
#include "MassEntitySubsystem.h"
#include "MassEntityView.h"
#endif
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Name.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Class.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_String.h"

UMassUnitBehaviorIntegration::UMassUnitBehaviorIntegration()
    : GASIntegration(nullptr)
{
}

UMassUnitBehaviorIntegration::~UMassUnitBehaviorIntegration()
{
}

void UMassUnitBehaviorIntegration::Initialize(UGASUnitIntegration* InGASIntegration)
{
    GASIntegration = InGASIntegration;
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitBehaviorIntegration: Initialized"));
}

void UMassUnitBehaviorIntegration::Deinitialize()
{
    // Clean up behavior tree components
    for (auto& Pair : EntityBTMap)
    {
        UBehaviorTreeComponent* BTComp = Pair.Value;
        if (BTComp)
        {
            BTComp->RemoveFromRoot();
        }
    }
    
    // Clean up blackboard components
    for (auto& Pair : EntityBBMap)
    {
        UBlackboardComponent* BBComp = Pair.Value;
        if (BBComp)
        {
            BBComp->RemoveFromRoot();
        }
    }
    
    // Clear maps
    EntityBTMap.Empty();
    EntityBBMap.Empty();
    
    // Clear references
    GASIntegration = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitBehaviorIntegration: Deinitialized"));
}


bool UMassUnitBehaviorIntegration::SetBehaviorTree(FMassUnitHandle UnitHandle, UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardData)
{
    return SetBehaviorTreeInternal(UnitHandle.EntityHandle, BehaviorTree, BlackboardData);
}

bool UMassUnitBehaviorIntegration::SetBehaviorTreeInternal(FMassEntityHandle Entity, UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardData)
{
    // Skip if not initialized
    if (!GASIntegration || !BehaviorTree || !BlackboardData)
    {
        return false;
    }
    
    // Create behavior tree component
    UBehaviorTreeComponent* BTComp = CreateBehaviorTreeForEntity(Entity, BehaviorTree, BlackboardData);
    if (!BTComp)
    {
        return false;
    }
    
    // Start behavior tree
    BTComp->StartTree(*BehaviorTree);
    
    UE_LOG(LogTemp, Log, TEXT("MassUnitBehaviorIntegration: Set behavior tree for entity %s"), *Entity.ToString());
    
    return true;
}

bool UMassUnitBehaviorIntegration::ExecuteBTTask(FMassUnitHandle UnitHandle, FGameplayTag TaskTag)
{
    return ExecuteBTTaskInternal(UnitHandle.EntityHandle, TaskTag);
}

bool UMassUnitBehaviorIntegration::ExecuteBTTaskInternal(FMassEntityHandle Entity, FGameplayTag TaskTag)
{
    // Skip if not initialized
    if (!GASIntegration)
    {
        return false;
    }
    
    // Get behavior tree component
    UBehaviorTreeComponent* BTComp = EntityBTMap.FindRef(Entity);
    if (!BTComp)
    {
        return false;
    }
    
    // In a real implementation, we would find and execute the task by tag
    // For this example, we'll just log that we're executing a task
    UE_LOG(LogTemp, Log, TEXT("MassUnitBehaviorIntegration: Executing BT task %s for entity %s"), 
        *TaskTag.ToString(), *Entity.ToString());
    
    // Update blackboard from entity data
    UpdateBlackboardFromEntity(Entity);
    
    return true;
}

UBehaviorTreeComponent* UMassUnitBehaviorIntegration::CreateBehaviorTreeForEntity(FMassEntityHandle Entity, UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardData)
{
    // Skip if already exists
    if (UBehaviorTreeComponent* ExistingBTComp = EntityBTMap.FindRef(Entity))
    {
        return ExistingBTComp;
    }
    
    // Create blackboard component
    UBlackboardComponent* BBComp = NewObject<UBlackboardComponent>(this);
    if (!BBComp)
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitBehaviorIntegration: Failed to create blackboard component"));
        return nullptr;
    }
    
    // Keep blackboard alive
    BBComp->AddToRoot();
    
    // Initialize blackboard
    BBComp->InitializeBlackboard(*BlackboardData);
    
    // Create behavior tree component
    UBehaviorTreeComponent* BTComp = NewObject<UBehaviorTreeComponent>(this);
    if (!BTComp)
    {
        UE_LOG(LogTemp, Error, TEXT("MassUnitBehaviorIntegration: Failed to create behavior tree component"));
        BBComp->RemoveFromRoot();
        return nullptr;
    }
    
    // Keep behavior tree alive
    BTComp->AddToRoot();
    
    // Set blackboard
    BTComp->SetBlackboardComponent(BBComp);
    
    // Add to maps
    EntityBTMap.Add(Entity, BTComp);
    EntityBBMap.Add(Entity, BBComp);
    
    // Initialize blackboard with entity data
    UpdateBlackboardFromEntity(Entity);
    
    return BTComp;
}

void UMassUnitBehaviorIntegration::UpdateBlackboardFromEntity(FMassEntityHandle Entity)
{
    // Skip if not initialized
    if (!GASIntegration)
    {
        return;
    }
    
    // Get blackboard component
    UBlackboardComponent* BBComp = EntityBBMap.FindRef(Entity);
    if (!BBComp)
    {
        return;
    }
    
    // Get entity subsystem from GAS integration
    UMassEntitySubsystem* EntitySubsystem = nullptr;
    if (GASIntegration->GetClass()->ImplementsInterface(UMassEntitySubsystemInterface::StaticClass()))
    {
        EntitySubsystem = IMassEntitySubsystemInterface::Execute_GetMassEntitySubsystem(GASIntegration);
    }
    
    // Skip if no entity subsystem
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
    
    // Update blackboard with entity data
    
    // Position
    if (EntityView.HasFragmentData<FTransformFragment>())
    {
        const FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
        BBComp->SetValueAsVector("Position", TransformFragment.GetTransform().GetLocation());
    }
    
    // State
    if (EntityView.HasFragmentData<FMassUnitStateFragment>())
    {
        const FMassUnitStateFragment& StateFragment = EntityView.GetFragmentData<FMassUnitStateFragment>();
        BBComp->SetValueAsEnum("State", static_cast<uint8>(StateFragment.CurrentState));
        BBComp->SetValueAsFloat("StateTime", StateFragment.StateTime);
        BBComp->SetValueAsName("UnitType", FName(*StateFragment.UnitType.ToString()));
        BBComp->SetValueAsInt("UnitLevel", StateFragment.UnitLevel);
    }
    
    // Target
    if (EntityView.HasFragmentData<FMassUnitTargetFragment>())
    {
        const FMassUnitTargetFragment& TargetFragment = EntityView.GetFragmentData<FMassUnitTargetFragment>();
        BBComp->SetValueAsVector("TargetLocation", TargetFragment.TargetLocation);
        BBComp->SetValueAsFloat("TargetPriority", TargetFragment.TargetPriority);
        BBComp->SetValueAsBool("HasTarget", TargetFragment.HasTarget());
    }
    
    // Team
    if (EntityView.HasFragmentData<FMassUnitTeamFragment>())
    {
        const FMassUnitTeamFragment& TeamFragment = EntityView.GetFragmentData<FMassUnitTeamFragment>();
        BBComp->SetValueAsInt("TeamID", TeamFragment.TeamID);
        BBComp->SetValueAsName("TeamFaction", FName(*TeamFragment.TeamFaction.ToString()));
    }
    
    // Attributes
    if (EntityView.HasFragmentData<FMassUnitAbilityFragment>())
    {
        const FMassUnitAbilityFragment& AbilityFragment = EntityView.GetFragmentData<FMassUnitAbilityFragment>();
        
        // Health
        FGameplayAttribute HealthAttribute = FGameplayAttribute::GetAttributeFromString("Health");
        if (HealthAttribute.IsValid() && AbilityFragment.AttributeValues.Contains(HealthAttribute))
        {
            BBComp->SetValueAsFloat("Health", AbilityFragment.AttributeValues[HealthAttribute]);
        }
        
        // Damage
        FGameplayAttribute DamageAttribute = FGameplayAttribute::GetAttributeFromString("Damage");
        if (DamageAttribute.IsValid() && AbilityFragment.AttributeValues.Contains(DamageAttribute))
        {
            BBComp->SetValueAsFloat("Damage", AbilityFragment.AttributeValues[DamageAttribute]);
        }
        
        // Speed
        FGameplayAttribute SpeedAttribute = FGameplayAttribute::GetAttributeFromString("Speed");
        if (SpeedAttribute.IsValid() && AbilityFragment.AttributeValues.Contains(SpeedAttribute))
        {
            BBComp->SetValueAsFloat("Speed", AbilityFragment.AttributeValues[SpeedAttribute]);
        }
    }
}

void UMassUnitBehaviorIntegration::UpdateEntityFromBlackboard(FMassEntityHandle Entity)
{
    // Skip if not initialized
    if (!GASIntegration)
    {
        return;
    }
    
    // Get blackboard component
    UBlackboardComponent* BBComp = EntityBBMap.FindRef(Entity);
    if (!BBComp)
    {
        return;
    }
    
    // Get entity subsystem from GAS integration
    UMassEntitySubsystem* EntitySubsystem = nullptr;
    if (GASIntegration->GetClass()->ImplementsInterface(UMassEntitySubsystemInterface::StaticClass()))
    {
        EntitySubsystem = IMassEntitySubsystemInterface::Execute_GetMassEntitySubsystem(GASIntegration);
    }
    
    // Skip if no entity subsystem
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
    
    // Update entity data from blackboard
    
    // Target
    if (EntityView.HasFragmentData<FMassUnitTargetFragment>())
    {
        FMassUnitTargetFragment& TargetFragment = EntityView.GetFragmentData<FMassUnitTargetFragment>();
        TargetFragment.TargetLocation = BBComp->GetValueAsVector("TargetLocation");
        TargetFragment.TargetPriority = BBComp->GetValueAsFloat("TargetPriority");
    }
    
    // State
    if (EntityView.HasFragmentData<FMassUnitStateFragment>())
    {
        FMassUnitStateFragment& StateFragment = EntityView.GetFragmentData<FMassUnitStateFragment>();
        StateFragment.CurrentState = static_cast<EMassUnitState>(BBComp->GetValueAsEnum("State"));
    }
}
