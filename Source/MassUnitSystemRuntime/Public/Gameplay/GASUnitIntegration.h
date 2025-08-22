// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassUnitEntityManager.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"

// Using native GAS classes

#include "Entity/MassEntityFallback.h"
#include "GASUnitIntegration.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;

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
    void Initialize(UMassUnitEntitySubsystem* InEntitySubsystem);
    
    /** Deinitialize the GAS integration */
    void Deinitialize();
    
    /** Get the ability system component for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UAbilitySystemComponent* GetAbilitySystemForEntity(FMassUnitHandle UnitHandle);
    
    /** Grant an ability to an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    FGameplayAbilitySpecHandle GrantAbility(FMassUnitHandle UnitHandle, TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);
    
    /** Activate an ability for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ActivateAbility(FMassUnitHandle UnitHandle, FGameplayTag AbilityTag);
    
    /** Apply a gameplay effect to an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool ApplyGameplayEffect(FMassUnitHandle UnitHandle, TSubclassOf<UGameplayEffect> EffectClass, AActor* Instigator = nullptr);
    
    // ...existing code...
    
    /** Internal method to get the ability system component for an entity */
    UAbilitySystemComponent* GetAbilitySystemForEntityInternal(FMassUnitEntityHandle Entity);
    
    /** Internal method to grant an ability to an entity */
    FGameplayAbilitySpecHandle GrantAbilityInternal(FMassUnitEntityHandle Entity, TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);
    
    /** Internal method to activate an ability for an entity */
    bool ActivateAbilityInternal(FMassUnitEntityHandle Entity, FGameplayTag AbilityTag);
    
    /** Internal method to apply a gameplay effect to an entity */
    bool ApplyGameplayEffectInternal(FMassUnitEntityHandle Entity, TSubclassOf<UGameplayEffect> EffectClass, AActor* Instigator = nullptr);
    
    // ...existing code...

private:
    /** Reference to the Mass Entity Subsystem */
    UPROPERTY(Transient)
    UMassUnitEntitySubsystem* EntitySubsystem;
    
    /** Map of entity handles to ability system components */
    TMap<FMassUnitEntityHandle, UAbilitySystemComponent*> EntityASCMap;
    
    /** Map of entity handles to attribute sets */
    TMap<FMassUnitEntityHandle, UAttributeSet*> EntityAttributeSetMap;
    
    /** Create an ability system component for an entity */
    UAbilitySystemComponent* CreateAbilitySystemForEntity(FMassUnitEntityHandle Entity);
    
    /** Sync entity data with ability system */
    void SyncEntityWithAbilitySystem(FMassUnitEntityHandle Entity);
    
    /** Sync ability system with entity data */
    void SyncAbilitySystemWithEntity(FMassUnitEntityHandle Entity);
};
