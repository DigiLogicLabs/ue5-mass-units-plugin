// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

// Include MassEntity types or fallback
#if WITH_MASSENTITY
class UMassEntitySubsystem;
#else
#include "Entity/MassEntityFallback.h"
#endif

#include "MassUnitSubsystem.generated.h"
class UMassUnitEntityManager;
class UFormationSystem;
class UMassUnitNavigationSystem;
class UNiagaraUnitSystem;
class UUnitMeshPool;
class UGASUnitIntegration;
class UMassUnitBehaviorIntegration;
class UUnitGameplayEventSystem;

/**
 * Game subsystem for managing unit-related operations in the Mass Unit System
 */
UCLASS()
class MASSUNITSYSTEMRUNTIME_API UMassUnitSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMassUnitSubsystem();
    virtual ~UMassUnitSubsystem();

    /** Initialize the subsystem */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
    /** Deinitialize the subsystem */
    virtual void Deinitialize() override;
    
    /** Tick function called every frame */
    void Tick(float DeltaTime);
    
    /** Get the unit entity manager */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UMassUnitEntityManager* GetUnitManager() const;
    
    /** Get the formation system */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UFormationSystem* GetFormationSystem() const;
    
    /** Get the navigation system */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UMassUnitNavigationSystem* GetNavigationSystem() const;
    
    /** Get the Niagara system */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UNiagaraUnitSystem* GetNiagaraSystem() const;
    
    /** Get the mesh pool */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UUnitMeshPool* GetMeshPool() const;
    
    /** Get the GAS integration */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UGASUnitIntegration* GetGASIntegration() const;
    
    /** Get the behavior integration */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UMassUnitBehaviorIntegration* GetBehaviorIntegration() const;
    
    /** Get the gameplay event system */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UUnitGameplayEventSystem* GetGameplayEventSystem() const;
    
    /** Get the entity subsystem */
    UMassEntitySubsystem* GetEntitySubsystem() const { return EntitySubsystem; }

private:
    /** Reference to the Mass Entity Subsystem */
    UPROPERTY(Transient)
    UMassEntitySubsystem* EntitySubsystem;
    
    /** Unit entity manager */
    UPROPERTY(Transient)
    UMassUnitEntityManager* UnitManager;
    
    /** Formation system */
    UPROPERTY(Transient)
    UFormationSystem* FormationSystem;
    
    /** Navigation system */
    UPROPERTY(Transient)
    UMassUnitNavigationSystem* NavigationSystem;
    
    /** Niagara system */
    UPROPERTY(Transient)
    UNiagaraUnitSystem* NiagaraSystem;
    
    /** Mesh pool */
    UPROPERTY(Transient)
    UUnitMeshPool* MeshPool;
    
    /** GAS integration */
    UPROPERTY(Transient)
    UGASUnitIntegration* GASIntegration;
    
    /** Behavior integration */
    UPROPERTY(Transient)
    UMassUnitBehaviorIntegration* BehaviorIntegration;
    
    /** Gameplay event system */
    UPROPERTY(Transient)
    UUnitGameplayEventSystem* GameplayEventSystem;
    
    /** Delegate handle for tick function */
    FDelegateHandle TickDelegateHandle;
};
