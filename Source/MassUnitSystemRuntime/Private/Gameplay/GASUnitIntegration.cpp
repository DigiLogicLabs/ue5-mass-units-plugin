// Copyright Your Company. All Rights Reserved.

#include "Gameplay/GASUnitIntegration.h"
#include "Entity/MassUnitFragments.h"

// Include MassEntity types or fallback
#if WITH_MASSENTITY
#include "MassEntitySubsystem.h"
#include "MassEntityView.h"
#endif
#include "AbilitySystemComponent.h"
#include "GameplayAbility.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AttributeSet.h"
#include "Engine/World.h"

UGASUnitIntegration::UGASUnitIntegration()
    : EntitySubsystem(nullptr)
{
}

UGASUnitIntegration::~UGASUnitIntegration()
{
}

void UGASUnitIntegration::Initialize(UMassEntitySubsystem* InEntitySubsystem)
{
    EntitySubsystem = InEntitySubsystem;
    
    UE_LOG(LogTemp, Log, TEXT("GASUnitIntegration: Initialized"));
}

void UGASUnitIntegration::Deinitialize()
{
    // Clean up ability system components
    for (auto& Pair : EntityASCMap)
    {
        UAbilitySystemComponent* ASC = Pair.Value;
        if (ASC)
        {
            ASC->RemoveFromRoot();
        }
    }
    
    // Clean up attribute sets
    for (auto& Pair : EntityAttributeSetMap)
    {
        UAttributeSet* AttributeSet = Pair.Value;
        if (AttributeSet)
        {
            AttributeSet->RemoveFromRoot();
        }
    }
    
    // Clear maps
    EntityASCMap.Empty();
    EntityAttributeSetMap.Empty();
    
    // Clear references
    EntitySubsystem = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("GASUnitIntegration: Deinitialized"));
}

UAbilitySystemComponent* UGASUnitIntegration::GetAbilitySystemForEntity(FMassEntityHandle Entity)
{
    // Check if entity already has an ASC
    if (UAbilitySystemComponent** ASCPtr = EntityASCMap.Find(Entity))
    {
        return *ASCPtr;
    }
    
    // Create a new ASC for this entity
    return CreateAbilitySystemForEntity(Entity);
}

FGameplayAbilitySpecHandle UGASUnitIntegration::GrantAbility(FMassEntityHandle Entity, TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    // Skip if no ability class
    if (!AbilityClass)
    {
        return FGameplayAbilitySpecHandle();
    }
    
    // Get ability system component
    UAbilitySystemComponent* ASC = GetAbilitySystemForEntity(Entity);
    if (!ASC)
    {
        return FGameplayAbilitySpecHandle();
    }
    
    // Create ability spec
    FGameplayAbilitySpec AbilitySpec(AbilityClass, Level);
    
    // Grant ability
    FGameplayAbilitySpecHandle AbilityHandle = ASC->GiveAbility(AbilitySpec);
    
    // Sync with entity data
    SyncAbilitySystemWithEntity(Entity);
    
    return AbilityHandle;
}

bool UGASUnitIntegration::ActivateAbility(FMassEntityHandle Entity, FGameplayTag AbilityTag)
{
    // Get ability system component
    UAbilitySystemComponent* ASC = GetAbilitySystemForEntity(Entity);
    if (!ASC)
    {
        return false;
    }
    
    // Activate ability by tag
    return ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
}

bool UGASUnitIntegration::ApplyGameplayEffect(FMassEntityHandle Entity, TSubclassOf<UGameplayEffect> EffectClass, AActor* Instigator)
{
    // Skip if no effect class
    if (!EffectClass)
    {
        return false;
    }
    
    // Get ability system component
    UAbilitySystemComponent* ASC = GetAbilitySystemForEntity(Entity);
    if (!ASC)
    {
        return false;
    }
    
    // Create effect context
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    if (Instigator)
    {
        EffectContext.AddInstigator(Instigator, Instigator);
    }
    
    // Create effect spec
    FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
    if (!EffectSpec.IsValid())
    {
        return false;
    }
    
    // Apply effect
    FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
    
    // Sync with entity data
    SyncAbilitySystemWithEntity(Entity);
    
    return EffectHandle.IsValid();
}

float UGASUnitIntegration::GetAttributeValue(FMassEntityHandle Entity, FGameplayAttribute Attribute)
{
    // Get ability system component
    UAbilitySystemComponent* ASC = GetAbilitySystemForEntity(Entity);
    if (!ASC)
    {
        return 0.0f;
    }
    
    // Get attribute value
    return ASC->GetNumericAttribute(Attribute);
}

bool UGASUnitIntegration::SetAttributeValue(FMassEntityHandle Entity, FGameplayAttribute Attribute, float Value)
{
    // Get ability system component
    UAbilitySystemComponent* ASC = GetAbilitySystemForEntity(Entity);
    if (!ASC)
    {
        return false;
    }
    
    // Set attribute value
    ASC->SetNumericAttributeBase(Attribute, Value);
    
    // Sync with entity data
    SyncAbilitySystemWithEntity(Entity);
    
    return true;
}

void UGASUnitIntegration::UpdateAttributes(FMassEntityHandle Entity, const TMap<FGameplayAttribute, float>& Attributes)
{
    // Get ability system component
    UAbilitySystemComponent* ASC = GetAbilitySystemForEntity(Entity);
    if (!ASC)
    {
        return;
    }
    
    // Update attributes
    for (const auto& Pair : Attributes)
    {
        ASC->SetNumericAttributeBase(Pair.Key, Pair.Value);
    }
    
    // Sync with entity data
    SyncAbilitySystemWithEntity(Entity);
}

UAbilitySystemComponent* UGASUnitIntegration::CreateAbilitySystemForEntity(FMassEntityHandle Entity)
{
    // Skip if not initialized
    if (!EntitySubsystem)
    {
        return nullptr;
    }
    
    // Get entity manager
    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    
    // Skip if entity is invalid
    if (!Entity.IsValid() || !EntityManager.IsEntityValid(Entity))
    {
        return nullptr;
    }
    
    // Create ability system component
    UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(this);
    if (!ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("GASUnitIntegration: Failed to create ability system component"));
        return nullptr;
    }
    
    // Keep ASC alive
    ASC->AddToRoot();
    
    // Create attribute set
    UAttributeSet* AttributeSet = NewObject<UAttributeSet>(ASC);
    if (AttributeSet)
    {
        // Keep attribute set alive
        AttributeSet->AddToRoot();
        
        // Add attribute set to ASC
        ASC->AddAttributeSetSubobject(AttributeSet);
        
        // Add to map
        EntityAttributeSetMap.Add(Entity, AttributeSet);
    }
    
    // Initialize ASC
    ASC->InitAbilityActorInfo(nullptr, nullptr);
    
    // Add to map
    EntityASCMap.Add(Entity, ASC);
    
    // Sync with entity data
    SyncEntityWithAbilitySystem(Entity);
    
    UE_LOG(LogTemp, Log, TEXT("GASUnitIntegration: Created ability system for entity %s"), *Entity.ToString());
    
    return ASC;
}

void UGASUnitIntegration::SyncEntityWithAbilitySystem(FMassEntityHandle Entity)
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
    if (!EntityView.HasFragmentData<FMassUnitAbilityFragment>())
    {
        return;
    }
    
    // Get ability fragment
    FMassUnitAbilityFragment& AbilityFragment = EntityView.GetFragmentData<FMassUnitAbilityFragment>();
    
    // Get ability system component
    UAbilitySystemComponent* ASC = EntityASCMap.FindRef(Entity);
    if (!ASC)
    {
        return;
    }
    
    // Update ability handles
    AbilityFragment.AbilityHandles.Empty();
    for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
    {
        AbilityFragment.AbilityHandles.Add(Spec.Handle);
    }
    
    // Update active effect tags
    AbilityFragment.ActiveEffectTags.Empty();
    FGameplayTagContainer ActiveTags;
    ASC->GetAggregatedTags(ActiveTags);
    for (const FGameplayTag& Tag : ActiveTags)
    {
        AbilityFragment.ActiveEffectTags.Add(Tag);
    }
    
    // Update attribute values
    AbilityFragment.AttributeValues.Empty();
    if (UAttributeSet* AttributeSet = EntityAttributeSetMap.FindRef(Entity))
    {
        // In a real implementation, we would iterate through all attributes in the attribute set
        // For this example, we'll just add some common attributes
        
        // Health
        FGameplayAttribute HealthAttribute = FGameplayAttribute::GetAttributeFromString("Health");
        if (HealthAttribute.IsValid())
        {
            AbilityFragment.AttributeValues.Add(HealthAttribute, ASC->GetNumericAttribute(HealthAttribute));
        }
        
        // Damage
        FGameplayAttribute DamageAttribute = FGameplayAttribute::GetAttributeFromString("Damage");
        if (DamageAttribute.IsValid())
        {
            AbilityFragment.AttributeValues.Add(DamageAttribute, ASC->GetNumericAttribute(DamageAttribute));
        }
        
        // Speed
        FGameplayAttribute SpeedAttribute = FGameplayAttribute::GetAttributeFromString("Speed");
        if (SpeedAttribute.IsValid())
        {
            AbilityFragment.AttributeValues.Add(SpeedAttribute, ASC->GetNumericAttribute(SpeedAttribute));
        }
    }
}

void UGASUnitIntegration::SyncAbilitySystemWithEntity(FMassEntityHandle Entity)
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
    if (!EntityView.HasFragmentData<FMassUnitAbilityFragment>())
    {
        return;
    }
    
    // Get ability fragment
    const FMassUnitAbilityFragment& AbilityFragment = EntityView.GetFragmentData<FMassUnitAbilityFragment>();
    
    // Get ability system component
    UAbilitySystemComponent* ASC = EntityASCMap.FindRef(Entity);
    if (!ASC)
    {
        return;
    }
    
    // Update attribute values
    for (const auto& Pair : AbilityFragment.AttributeValues)
    {
        ASC->SetNumericAttributeBase(Pair.Key, Pair.Value);
    }
}
