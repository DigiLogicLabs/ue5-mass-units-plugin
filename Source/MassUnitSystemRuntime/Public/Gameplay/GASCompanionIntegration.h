// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Entity/MassUnitEntityManager.h"

// Check if GASCompanion is available
#if WITH_GASCOMPANION
#include "Abilities/GSCAbilitySystemComponent.h"
#include "Abilities/GSCAbilitySet.h"
#endif

#include "GASCompanionIntegration.generated.h"

class UGASUnitIntegration;
class UBehaviorTree;
class UBlackboardData;

// Forward declare GASCompanion classes if not available
#if !WITH_GASCOMPANION
class UGSCAbilitySystemComponent;
class UGSCAbilitySet;
#endif

/**
 * Specific integration with GASCompanion for the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UGASCompanionIntegration : public UObject
{
    GENERATED_BODY()

public:
    UGASCompanionIntegration();
    virtual ~UGASCompanionIntegration();

    /** Initialize the GASCompanion integration */
    void Initialize(UGASUnitIntegration* InGASIntegration);
    
    /** Deinitialize the GASCompanion integration */
    void Deinitialize();
    
    /** Create a GASCompanion-compatible unit */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool CreateGSCCompatibleUnit(FMassUnitHandle UnitHandle);
    
    /** Apply a GASCompanion ability set to an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ApplyGSCAbilitySet(FMassUnitHandle UnitHandle, UGSCAbilitySet* AbilitySet);
    
    /** Set behavior tree for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool SetBehaviorTree(FMassUnitHandle UnitHandle, UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardData);
    
    /** Execute a behavior tree task for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ExecuteBTTask(FMassUnitHandle UnitHandle, FGameplayTag TaskTag);
    
    /** Internal method to create a GASCompanion-compatible unit */
    bool CreateGSCCompatibleUnitInternal(FMassEntityHandle Entity);
    
    /** Internal method to apply a GASCompanion ability set to an entity */
    bool ApplyGSCAbilitySetInternal(FMassEntityHandle Entity, UGSCAbilitySet* AbilitySet);
    
    /** Internal method to set behavior tree for an entity */
    bool SetBehaviorTreeInternal(FMassEntityHandle Entity, UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardData);
    
    /** Internal method to execute a behavior tree task for an entity */
    bool ExecuteBTTaskInternal(FMassEntityHandle Entity, FGameplayTag TaskTag);

private:
    /** Reference to the GAS integration */
    UPROPERTY(Transient)
    UGASUnitIntegration* GASIntegration;
    
    /** Map of entity handles to behavior tree components */
    TMap<FMassEntityHandle, class UBehaviorTreeComponent*> EntityBTMap;
    
    /** Map of entity handles to blackboard components */
    TMap<FMassEntityHandle, class UBlackboardComponent*> EntityBBMap;
    
    /** Create behavior tree component for an entity */
    class UBehaviorTreeComponent* CreateBehaviorTreeForEntity(FMassEntityHandle Entity, UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardData);
    
    /** Update blackboard from entity data */
    void UpdateBlackboardFromEntity(FMassEntityHandle Entity);
    
    /** Update entity data from blackboard */
    void UpdateEntityFromBlackboard(FMassEntityHandle Entity);
};
