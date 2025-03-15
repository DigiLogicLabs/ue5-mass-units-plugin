// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MassUnitSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassUnitEntityManager;
class UFormationSystem;

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
    UMassUnitEntityManager* GetUnitManager() const { return UnitManager; }
    
    /** Get the formation system */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UFormationSystem* GetFormationSystem() const { return FormationSystem; }
    
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
    
    /** Delegate handle for tick function */
    FDelegateHandle TickDelegateHandle;
};
