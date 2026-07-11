// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MassUnitSystemSettings.generated.h"

class UNiagaraSystem;
class UStaticMesh;
class UUnitConfigDataAsset;

/** Project-wide defaults for the Mass Unit System. */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Mass Unit System"))
class MASSUNITSYSTEMRUNTIME_API UMassUnitSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMassUnitSystemSettings();

	/** Hard cap enforced by the Blueprint-facing unit manager. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Performance", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxUnits = 10000;

	/** Maximum number of units represented by individual skeletal mesh components. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Performance", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxSkeletalMeshUnits = 100;

	/** Maximum queued navigation requests submitted per world tick. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Performance", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxPathRequestsPerFrame = 100;

	/** Minimum interval between GPU/instanced representation uploads. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Performance", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float VisualUpdateInterval = 0.033f;

	/** Distances, in centimeters, at which a unit advances to the next visual LOD. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "LOD", meta = (ForceUnits = "cm"))
	TArray<float> LODDistanceThresholds;

	/** Units with a skeletal mesh use it inside this distance when pool capacity permits. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "LOD", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float SkeletalMeshDistance = 300.0f;

	/** Units beyond this distance are excluded from visual representation. Zero disables distance culling. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "LOD", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float MaxVisibleDistance = 10000.0f;

	/** Optional Niagara system that consumes the documented Unit* array parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Rendering")
	TSoftObjectPtr<UNiagaraSystem> DefaultNiagaraSystem;

	/** Use instanced static meshes when no Niagara system is configured or can be loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Rendering")
	bool bEnableInstancedMeshFallback = true;

	/** Optional final fallback mesh. If unset, the engine cube is used so new installs remain visible. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Rendering")
	TSoftObjectPtr<UStaticMesh> FallbackStaticMesh;

	/** If no nav data exists, use a direct two-point path instead of rejecting movement requests. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Navigation")
	bool bFallbackToDirectPath = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Debug")
	bool bEnableDebugVisualization = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Debug", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float DebugVisualizationDuration = 0.0f;

	/** Optional project catalog. These assets are not spawned automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Units")
	TArray<TSoftObjectPtr<UUnitConfigDataAsset>> UnitConfigurations;

	/** Optional default catalog entry for project-owned selection workflows. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Units")
	TSoftObjectPtr<UUnitConfigDataAsset> DefaultUnitConfiguration;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System")
	static UMassUnitSystemSettings* Get();
};
