// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassUnitEntityManager.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"

// Using native GAS classes

#include "GASUnitIntegration.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
// Forward declare UMassEntitySubsystem if not including the full header
#if !WITH_MASSENTITY
class UMassEntitySubsystem;
#else
#include "MassEntitySubsystem.h" // Include full definition when MassEntity is enabled
#endif
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
    void Initialize(UMassEntitySubsystem* InEntitySubsystem);
    
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
    
    /** Get an attribute value for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    float GetAttributeValue(FMassUnitHandle UnitHandle, FGameplayAttribute Attribute);
    
    /** Set an attribute value for an entity */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    bool SetAttributeValue(FMassUnitHandle UnitHandle, FGameplayAttribute Attribute, float Value);
    
    /** Internal method to get the ability system component for an entity */
    UAbilitySystemComponent* GetAbilitySystemForEntityInternal(FMassEntityHandle Entity);
    
    /** Internal method to grant an ability to an entity */
    FGameplayAbilitySpecHandle GrantAbilityInternal(FMassEntityHandle Entity, TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);
    
    /** Internal method to activate an ability for an entity */
    bool ActivateAbilityInternal(FMassEntityHandle Entity, FGameplayTag AbilityTag);
    
    /** Internal method to apply a gameplay effect to an entity */
    bool ApplyGameplayEffectInternal(FMassEntityHandle Entity, TSubclassOf<UGameplayEffect> EffectClass, AActor* Instigator = nullptr);
    
    /** Internal method to get an attribute value for an entity */
    float GetAttributeValueInternal(FMassEntityHandle Entity, FGameplayAttribute Attribute);
    
    /** Internal method to set an attribute value for an entity */
    bool SetAttributeValueInternal(FMassEntityHandle Entity, FGameplayAttribute Attribute, float Value);
    
    /** Update attributes from entity data */
    void UpdateAttributes(FMassEntityHandle Entity, const TMap<FGameplayAttribute, float>& Attributes);

private:
    /** Reference to the Mass Entity Subsystem */
    UPROPERTY(Transient)
    UMassEntitySubsystem* EntitySubsystem;
    
    /** Map of entity handles to ability system components */
    TMap<FMassEntityHandle, UAbilitySystemComponent*> EntityASCMap;
    
    /** Map of entity handles to attribute sets */
    TMap<FMassEntityHandle, UAttributeSet*> EntityAttributeSetMap;
    
    /** Create an ability system component for an entity */
    UAbilitySystemComponent* CreateAbilitySystemForEntity(FMassEntityHandle Entity);
    
    /** Sync entity data with ability system */
    void SyncEntityWithAbilitySystem(FMassEntityHandle Entity);
    
    /** Sync ability system with entity data */
    void SyncAbilitySystemWithEntity(FMassEntityHandle Entity);
};
