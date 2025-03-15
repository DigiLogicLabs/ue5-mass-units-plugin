// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "GameplayAbilitySpec.h"
#include "GASUnitIntegration.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
class UMassEntitySubsystem;
class UGSCAbilitySystemComponent;
class UGSCAttributeSet;

/**
 * Bridge between Mass Entity System and Gameplay Ability System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UGASUnitIntegration : public UObject
{
    GENERATED_BODY()

public:
    UGASUnitIntegration();
    virtual ~UGASUnitIntegration();

    /** Initialize the GAS integration */
    void Initialize(UMassEntitySubsystem* InEntitySubsystem);
    
    /** Deinitialize the GAS integration */
    void Deinitialize();
    
    /** Get the ability system component for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UAbilitySystemComponent* GetAbilitySystemForEntity(FMassEntityHandle Entity);
    
    /** Grant an ability to an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    FGameplayAbilitySpecHandle GrantAbility(FMassEntityHandle Entity, TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);
    
    /** Activate an ability for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ActivateAbility(FMassEntityHandle Entity, FGameplayTag AbilityTag);
    
    /** Apply a gameplay effect to an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ApplyGameplayEffect(FMassEntityHandle Entity, TSubclassOf<UGameplayEffect> EffectClass, AActor* Instigator = nullptr);
    
    /** Get an attribute value for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    float GetAttributeValue(FMassEntityHandle Entity, FGameplayAttribute Attribute);
    
    /** Set an attribute value for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool SetAttributeValue(FMassEntityHandle Entity, FGameplayAttribute Attribute, float Value);
    
    /** Update attributes from entity data */
    void UpdateAttributes(FMassEntityHandle Entity, const TMap<FGameplayAttribute, float>& Attributes);

private:
    /** Reference to the Mass Entity Subsystem */
    UPROPERTY(Transient)
    UMassEntitySubsystem* EntitySubsystem;
    
    /** Map of entity handles to ability system components */
    TMap<FMassEntityHandle, UGSCAbilitySystemComponent*> EntityASCMap;
    
    /** Map of entity handles to attribute sets */
    TMap<FMassEntityHandle, UGSCAttributeSet*> EntityAttributeSetMap;
    
    /** Create an ability system component for an entity */
    UGSCAbilitySystemComponent* CreateAbilitySystemForEntity(FMassEntityHandle Entity);
    
    /** Sync entity data with ability system */
    void SyncEntityWithAbilitySystem(FMassEntityHandle Entity);
    
    /** Sync ability system with entity data */
    void SyncAbilitySystemWithEntity(FMassEntityHandle Entity);
};
