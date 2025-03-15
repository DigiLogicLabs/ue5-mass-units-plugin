// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassUnitEntityManager.h"

#include "MassUnitBehaviorIntegration.generated.h"

class UGASUnitIntegration;
class UBehaviorTree;
class UBlackboardData;

/**
 * Behavior tree integration for the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UMassUnitBehaviorIntegration : public UObject
{
    GENERATED_BODY()

public:
    UMassUnitBehaviorIntegration();
    virtual ~UMassUnitBehaviorIntegration();

    /** Initialize the behavior integration */
    void Initialize(UGASUnitIntegration* InGASIntegration);
    
    /** Deinitialize the behavior integration */
    void Deinitialize();
    
    /** Set behavior tree for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool SetBehaviorTree(FMassUnitHandle UnitHandle, UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardData);
    
    /** Execute a behavior tree task for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ExecuteBTTask(FMassUnitHandle UnitHandle, FGameplayTag TaskTag);
    
    
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
