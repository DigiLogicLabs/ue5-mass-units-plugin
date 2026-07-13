// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassUnitEntityManager.h"
#include "NiagaraUnitSystem.generated.h"

class UInstancedStaticMeshComponent;
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

	/** Number of dynamic ISM components currently allocated by the asset-free/static-mesh fallback. */
	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering|Diagnostics")
	int32 GetInstancedMeshComponentCount() const { return InstancedMeshComponents.Num(); }

	/** Total instances currently submitted through the ISM fallback. Returns zero when Niagara is active. */
	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering|Diagnostics")
	int32 GetInstancedMeshInstanceCount() const;

	/** Increments only when instanced-rendering slots are added or removed, not while units move. */
	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering|Diagnostics")
	int32 GetInstancedMeshTopologyRevision() const { return InstancedMeshTopologyRevision; }

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
	TMap<TObjectPtr<UStaticMesh>, TObjectPtr<UInstancedStaticMeshComponent>> InstancedMeshComponents;

	int32 CurrentLODLevel = 0;
	int32 MaxUnits = 10000;
	int32 InstancedMeshTopologyRevision = 0;
	float UpdateFrequency = 0.033f;
	float LastUpdateTime = -BIG_NUMBER;
	bool bEnableInstancedFallback = true;

	void CreateNiagaraSystem();
	void UpdateNiagaraData(const TArray<FMassUnitEntityHandle>& Entities);
	void UpdateInstancedMeshData(const TArray<FMassUnitEntityHandle>& Entities);
	void SynchronizeInstancedMeshComponent(UInstancedStaticMeshComponent* Component, const TArray<FTransform>& WorldTransforms);
	UInstancedStaticMeshComponent* GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh);
};
