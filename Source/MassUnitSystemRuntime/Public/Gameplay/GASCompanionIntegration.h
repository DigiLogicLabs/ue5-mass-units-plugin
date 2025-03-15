// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "GASCompanionIntegration.generated.h"

class UGASUnitIntegration;
class UGSCAbilitySystemComponent;
class UGSCAbilitySet;
class UBehaviorTree;
class UBlackboardData;

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
    bool CreateGSCCompatibleUnit(FMassEntityHandle Entity);
    
    /** Apply a GASCompanion ability set to an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ApplyGSCAbilitySet(FMassEntityHandle Entity, UGSCAbilitySet* AbilitySet);
    
    /** Set behavior tree for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool SetBehaviorTree(FMassEntityHandle Entity, UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardData);
    
    /** Execute a behavior tree task for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ExecuteBTTask(FMassEntityHandle Entity, FGameplayTag TaskTag);

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
