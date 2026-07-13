// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassUnitEntityManager.h"
#include "GameFramework/Actor.h"
#include "MassUnitSpawner.generated.h"

class UArrowComponent;
class USceneComponent;
class UUnitTemplate;

/**
 * Placeable, actor-light bootstrap for repeatable Mass Unit spawning.
 *
 * The actor owns only stable Mass handles. The units themselves remain native
 * Mass entities, and the actor performs no per-frame work.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Mass Unit Spawner"))
class MASSUNITSYSTEMRUNTIME_API AMassUnitSpawner : public AActor
{
	GENERATED_BODY()

public:
	AMassUnitSpawner();

	/** Optional data-driven template. When empty, the plugin's visible asset-free default is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Units")
	TObjectPtr<UUnitTemplate> UnitTemplate = nullptr;

	/** Number of native Mass entities created by Spawn Units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Units", meta = (ClampMin = "1", UIMin = "1", UIMax = "1000"))
	int32 UnitCount = 25;

	/** Maximum units per grid row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Layout", meta = (ClampMin = "1", UIMin = "1"))
	int32 Columns = 5;

	/** Actor-local distance between grid rows (X) and columns (Y). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Layout", meta = (ClampMin = "1.0", ForceUnits = "cm"))
	FVector2D UnitSpacing = FVector2D(150.0f, 150.0f);

	/** Actor-local height added to every spawn point. Fifty centimeters centers the default engine cube on a floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Layout", meta = (ForceUnits = "cm"))
	float SpawnHeight = 50.0f;

	/** Create the grid automatically when gameplay begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Lifecycle")
	bool bSpawnOnBeginPlay = true;

	/** Command every spawned unit immediately after the automatic spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Movement", meta = (EditCondition = "bSpawnOnBeginPlay"))
	bool bMoveOnBeginPlay = true;

	/** Actor-local offset applied to each unit's own spawn point, preserving the grid while it moves. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Movement", meta = (EditCondition = "bMoveOnBeginPlay", ForceUnits = "cm"))
	FVector DestinationOffset = FVector(1500.0f, 0.0f, 0.0f);

	/** Request navmesh paths instead of writing direct destinations. Missing nav data still uses the project fallback setting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Movement")
	bool bUseNavigation = false;

	/** Distance from a destination at which movement stops. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Movement", meta = (ClampMin = "1.0", ForceUnits = "cm"))
	float AcceptanceRadius = 50.0f;

	/** Prevent duplicate autonomous simulation on network clients. Standalone and listen-server quick starts still spawn normally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Networking")
	bool bSpawnOnAuthorityOnly = true;

	/** Destroy only the Mass entities created by this actor when the actor or world ends play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Lifecycle")
	bool bDestroySpawnedUnitsOnEndPlay = true;

	/** Draw spawn boxes and command lines when Spawn Units or a command function is called. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Debug")
	bool bEnableVisualDebug = false;

	/** Lifetime of one-shot debug geometry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit Spawner|Debug", meta = (EditCondition = "bEnableVisualDebug", ClampMin = "0.0", ForceUnits = "s"))
	float DebugDuration = 10.0f;

	/** Handles created and owned by this spawner. Invalid/destroyed handles are filtered by the getter functions. */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Mass Unit Spawner|Runtime")
	TArray<FMassUnitHandle> SpawnedUnits;

	/** Appends one configured grid of units and returns the successfully created handles. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Spawner", meta = (ReturnDisplayName = "New Units"))
	TArray<FMassUnitHandle> SpawnUnits();

	/** Moves valid owned units by the same actor-local offset, preserving their current spacing. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Spawner", meta = (ReturnDisplayName = "Commanded Unit Count"))
	int32 MoveSpawnedUnitsByOffset(FVector LocalOffset);

	/** Moves valid owned units around a world-space center, optionally preserving their current spacing. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Spawner", meta = (ReturnDisplayName = "Commanded Unit Count"))
	int32 CommandSpawnedUnitsToLocation(FVector DestinationCenter, bool bPreserveSpacing = true);

	/** Destroys valid Mass entities created by this actor and clears its handles. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Spawner")
	void DestroySpawnedUnits();

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Spawner")
	TArray<FMassUnitHandle> GetValidSpawnedUnits() const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Spawner")
	int32 GetValidSpawnedUnitCount() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Mass Unit Spawner", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	/** Shows the default local movement direction in the editor viewport. */
	UPROPERTY(VisibleAnywhere, Category = "Mass Unit Spawner", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> DirectionArrow = nullptr;

	FVector CalculateLocalGridOffset(int32 UnitIndex, int32 TotalUnits) const;
	bool CommandUnit(FMassUnitHandle UnitHandle, const FVector& Destination) const;
	void DrawSpawnDebug(const FVector& Location) const;
	void DrawCommandDebug(const FVector& Start, const FVector& Destination) const;
};
