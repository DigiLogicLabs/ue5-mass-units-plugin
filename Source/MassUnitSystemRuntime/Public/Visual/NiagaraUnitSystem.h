// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraUnitSystem.generated.h"

class UMassEntitySubsystem;
class UVertexAnimationManager;

/**
 * Niagara-based system for rendering units in the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UNiagaraUnitSystem : public UObject
{
    GENERATED_BODY()

public:
    UNiagaraUnitSystem();
    virtual ~UNiagaraUnitSystem();

    /** Initialize the Niagara unit system */
    void Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem);
    
    /** Deinitialize the Niagara unit system */
    void Deinitialize();
    
    /** Update unit visuals */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    void UpdateUnitVisuals(const TArray<FMassEntityHandle>& Entities);
    
    /** Set LOD level */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    void SetLODLevel(int32 LODLevel);
    
    /** Get the Niagara component */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UNiagaraComponent* GetNiagaraComponent() const { return NiagaraComponent; }
    
    /** Get the vertex animation manager */
    UVertexAnimationManager* GetVertexAnimationManager() const { return VertexAnimationManager; }

private:
    /** Reference to the world */
    UPROPERTY(Transient)
    UWorld* World;
    
    /** Reference to the Mass Entity Subsystem */
    UPROPERTY(Transient)
    UMassEntitySubsystem* EntitySubsystem;
    
    /** Niagara system asset */
    UPROPERTY(Transient)
    UNiagaraSystem* NiagaraSystemAsset;
    
    /** Niagara component */
    UPROPERTY(Transient)
    UNiagaraComponent* NiagaraComponent;
    
    /** Vertex animation manager */
    UPROPERTY(Transient)
    UVertexAnimationManager* VertexAnimationManager;
    
    /** Current LOD level */
    int32 CurrentLODLevel;
    
    /** Maximum number of units */
    UPROPERTY(EditAnywhere, Category = "Niagara")
    int32 MaxUnits = 10000;
    
    /** Update frequency */
    UPROPERTY(EditAnywhere, Category = "Niagara")
    float UpdateFrequency = 0.033f; // ~30 fps
    
    /** Last update time */
    float LastUpdateTime;
    
    /** Create the Niagara system */
    void CreateNiagaraSystem();
    
    /** Update unit data in Niagara */
    void UpdateUnitData(const TArray<FMassEntityHandle>& Entities);
};
