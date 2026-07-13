// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassUnitEntityManager.h"
#include "NiagaraUnitSystem.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UMassEntitySubsystem;
class UNiagaraComponent;
class UNiagaraSystem;
class UStaticMesh;
class UVertexAnimationManager;

/** GPU representation uploader with an asset-free instanced-mesh fallback. */
UCLASS(BlueprintType)
class MASSUNITSYSTEMRUNTIME_API UNiagaraUnitSystem : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem);
	void Deinitialize();
	void UpdateUnitVisuals(const TArray<FMassUnitEntityHandle>& Entities);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Rendering")
	void UpdateUnitVisualsByHandles(const TArray<FMassUnitHandle>& UnitHandles);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Rendering")
	void SetLODLevel(int32 LODLevel);

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering")
	UNiagaraComponent* GetNiagaraComponent() const { return NiagaraComponent; }

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering")
	bool IsUsingNiagara() const { return NiagaraComponent != nullptr; }

	/** Number of HISM components currently allocated by the asset-free/static-mesh fallback. */
	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering|Diagnostics")
	int32 GetInstancedMeshComponentCount() const { return InstancedMeshComponents.Num(); }

	/** Total instances currently submitted through the HISM fallback. Returns zero when Niagara is active. */
	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering|Diagnostics")
	int32 GetInstancedMeshInstanceCount() const;

	UVertexAnimationManager* GetVertexAnimationManager() const { return VertexAnimationManager; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UWorld> World = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMassEntitySubsystem> EntitySubsystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> NiagaraSystemAsset = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UVertexAnimationManager> VertexAnimationManager = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> FallbackStaticMesh = nullptr;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UStaticMesh>, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> InstancedMeshComponents;

	int32 CurrentLODLevel = 0;
	int32 MaxUnits = 10000;
	float UpdateFrequency = 0.033f;
	float LastUpdateTime = -BIG_NUMBER;
	bool bEnableInstancedFallback = true;

	void CreateNiagaraSystem();
	void UpdateNiagaraData(const TArray<FMassUnitEntityHandle>& Entities);
	void UpdateInstancedMeshData(const TArray<FMassUnitEntityHandle>& Entities);
	UHierarchicalInstancedStaticMeshComponent* GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh);
};
