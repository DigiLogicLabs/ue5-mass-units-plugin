// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassUnitEntityManager.h"
#include "MassUnitCrowdSystem.generated.h"

class UMassEntitySubsystem;
class UMassUnitNavigationSystem;

/** Designer-facing controls for one lightweight crowd group. */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitCrowdConfig
{
	GENERATED_BODY()

	/** Maximum world-space radius used when choosing random destinations around the group center. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Wander", meta = (ClampMin = "1.0", ForceUnits = "cm"))
	float WanderRadius = 1500.0f;

	/** Minimum distance of a newly selected destination from the unit's current location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Wander", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float MinWanderDistance = 300.0f;

	/** Minimum pause after reaching a destination before selecting another one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Wander", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float MinIdleTime = 0.5f;

	/** Maximum pause after reaching a destination before selecting another one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Wander", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float MaxIdleTime = 2.0f;

	/** Re-evaluate a destination after this time so blocked units cannot remain stuck forever. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Wander", meta = (ClampMin = "0.1", ForceUnits = "s"))
	float MaxMoveTime = 8.0f;

	/** Lowest random movement-speed multiplier applied when a new destination is selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Wander", meta = (ClampMin = "0.05", ClampMax = "4.0"))
	float MinMoveSpeedMultiplier = 0.8f;

	/** Highest random movement-speed multiplier applied when a new destination is selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Wander", meta = (ClampMin = "0.05", ClampMax = "4.0"))
	float MaxMoveSpeedMultiplier = 1.2f;

	/** Enables spatial-hash separation steering without adding collision or an Actor per unit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Avoidance")
	bool bEnableSeparation = true;

	/** Neighbor radius used by lightweight separation steering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Avoidance", meta = (EditCondition = "bEnableSeparation", ClampMin = "1.0", ForceUnits = "cm"))
	float SeparationRadius = 140.0f;

	/** Separation influence relative to the desired travel direction. Zero disables steering; one gives equal influence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Avoidance", meta = (EditCondition = "bEnableSeparation", ClampMin = "0.0", ClampMax = "4.0"))
	float SeparationWeight = 1.25f;

	/** Enables occasional paired social pauses. Interactions broadcast a Blueprint event and do not imply combat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Interaction")
	bool bEnableInteractions = true;

	/** Chance from zero to one that an idle decision becomes an interaction when a neighbor is available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Interaction", meta = (EditCondition = "bEnableInteractions", ClampMin = "0.0", ClampMax = "1.0"))
	float InteractionChance = 0.15f;

	/** Maximum distance at which two units can begin a lightweight interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Interaction", meta = (EditCondition = "bEnableInteractions", ClampMin = "1.0", ForceUnits = "cm"))
	float InteractionRadius = 275.0f;

	/** Minimum duration of a paired interaction pause. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Interaction", meta = (EditCondition = "bEnableInteractions", ClampMin = "0.0", ForceUnits = "s"))
	float MinInteractionTime = 0.75f;

	/** Maximum duration of a paired interaction pause. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Interaction", meta = (EditCondition = "bEnableInteractions", ClampMin = "0.0", ForceUnits = "s"))
	float MaxInteractionTime = 2.0f;

	/** Units farther than this from every player observer sleep until a player returns. Zero disables simulation sleeping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float MaxSimulationDistance = 15000.0f;

	/** Stable seed used with each native entity handle so the same setup produces repeatable choices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Randomness", meta = (ClampMin = "0"))
	int32 RandomSeed = 1337;

	/** Draw group bounds, destination arrows, and interaction links as decisions occur. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Debug")
	bool bEnableVisualDebug = false;
};

/** Snapshot of the most recent budgeted crowd update. */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitCrowdStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
	int32 RegisteredGroups = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
	int32 RegisteredUnits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
	int32 ActiveUnits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
	int32 SleepingUnits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
	int32 UnitsUpdated = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
	int32 DestinationsAssigned = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
	int32 InteractionsStarted = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
	int32 NeighborChecks = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMassUnitCrowdInteractionSignature,
	FMassUnitHandle, UnitA,
	FMassUnitHandle, UnitB);

/**
 * World-owned, timer-driven behavior LOD for lightweight ambient crowds.
 * Smooth movement remains in the native Mass processor; this service only
 * performs budgeted decisions, spatial-hash steering, and interaction events.
 */
UCLASS(BlueprintType)
class MASSUNITSYSTEMRUNTIME_API UMassUnitCrowdSystem : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(
		UWorld* InWorld,
		UMassEntitySubsystem* InEntitySubsystem,
		UMassUnitEntityManager* InUnitManager,
		UMassUnitNavigationSystem* InNavigationSystem);
	void Deinitialize();

	/** Registers valid handles as one independently configurable crowd group. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Crowd", meta = (ReturnDisplayName = "Crowd Group Handle"))
	int32 RegisterCrowdGroup(
		const TArray<FMassUnitHandle>& UnitHandles,
		FVector Center,
		const FMassUnitCrowdConfig& Config,
		bool bUseNavigation = false,
		float AcceptanceRadius = 50.0f);

	/** Removes a crowd group. Optionally clears movement and restores each unit's original speed. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Crowd")
	bool UnregisterCrowdGroup(int32 CrowdGroupHandle, bool bStopUnits = true);

	/** Pauses or resumes low-frequency behavior for a group without destroying its units. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Crowd")
	bool SetCrowdGroupPaused(int32 CrowdGroupHandle, bool bPaused, bool bStopUnits = true);

	/** Moves the center used for future random destinations. Existing destinations are not changed. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Crowd")
	bool SetCrowdGroupCenter(int32 CrowdGroupHandle, FVector NewCenter);

	/** Immediately gives active units in one group a fresh decision, bypassing the normal per-update budget. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Crowd")
	bool ForceCrowdGroupUpdate(int32 CrowdGroupHandle);

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Crowd")
	bool IsCrowdGroupRegistered(int32 CrowdGroupHandle) const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Crowd")
	int32 GetCrowdGroupUnitCount(int32 CrowdGroupHandle) const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Crowd|Diagnostics")
	FMassUnitCrowdStats GetCrowdStats() const { return LastStats; }

	/** Fired for non-combat ambient interactions; projects can attach audio, animation, or gameplay reactions. */
	UPROPERTY(BlueprintAssignable, Category = "Mass Unit System|Crowd|Events")
	FMassUnitCrowdInteractionSignature OnCrowdInteractionStarted;

private:
	struct FCrowdGroup
	{
		int32 Handle = INDEX_NONE;
		FVector Center = FVector::ZeroVector;
		FMassUnitCrowdConfig Config;
		TArray<FMassUnitEntityHandle> Units;
		float AcceptanceRadius = 50.0f;
		bool bUseNavigation = false;
		bool bPaused = false;
	};

	struct FSpatialEntry
	{
		FMassUnitEntityHandle Entity;
		FVector Location = FVector::ZeroVector;
		int32 GroupHandle = INDEX_NONE;
	};

	UPROPERTY(Transient)
	TObjectPtr<UWorld> World = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMassEntitySubsystem> EntitySubsystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMassUnitEntityManager> UnitManager = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMassUnitNavigationSystem> NavigationSystem = nullptr;

	TMap<int32, FCrowdGroup> Groups;
	TMap<FMassUnitEntityHandle, int32> UnitToGroup;
	FTimerHandle UpdateTimer;
	FMassUnitCrowdStats LastStats;
	int32 NextGroupHandle = 1;
	int32 UpdateCursor = 0;
	int32 MaxUnitsPerUpdate = 500;
	float UpdateInterval = 0.1f;
	float SpatialCellSize = 200.0f;
	bool bUpdatingCrowds = false;
	TArray<float> SimulationLODDistances;
	TArray<float> SimulationLODIntervalMultipliers;

	void StartTimerIfNeeded();
	void StopTimerIfIdle();
	void UpdateCrowds();
	void PruneInvalidUnits();
	void BuildSpatialData(
		TArray<FSpatialEntry>& OutEntries,
		TMap<FIntPoint, TArray<int32>>& OutCells,
		TMap<FMassUnitEntityHandle, int32>& OutEntryByEntity) const;
	void BuildObserverLocations(TArray<FVector>& OutObserverLocations) const;
	void RefreshPopulationStats(const TArray<FSpatialEntry>& Entries);
	void UpdateUnit(
		const FSpatialEntry& Entry,
		FCrowdGroup& Group,
		const TArray<FSpatialEntry>& Entries,
		const TMap<FIntPoint, TArray<int32>>& Cells,
		const TMap<FMassUnitEntityHandle, int32>& EntryByEntity,
		const TArray<FVector>& ObserverLocations,
		float CurrentTime,
		bool bForceDecision);
	FVector CalculateSeparation(
		const FSpatialEntry& Entry,
		const FCrowdGroup& Group,
		const TArray<FSpatialEntry>& Entries,
		const TMap<FIntPoint, TArray<int32>>& Cells);
	FMassUnitEntityHandle FindInteractionPartner(
		const FSpatialEntry& Entry,
		const FCrowdGroup& Group,
		const TArray<FSpatialEntry>& Entries,
		const TMap<FIntPoint, TArray<int32>>& Cells,
		float CurrentTime);
	void BeginInteraction(
		FMassUnitEntityHandle UnitA,
		FMassUnitEntityHandle UnitB,
		FCrowdGroup& Group,
		float CurrentTime,
		FRandomStream& RandomStream);
	bool AssignRandomDestination(
		FMassUnitEntityHandle Entity,
		FCrowdGroup& Group,
		float CurrentTime,
		float LODIntervalMultiplier,
		FRandomStream& RandomStream);
	void ClearMovement(FMassUnitEntityHandle Entity, bool bSetIdle) const;
	void SetUnitSleeping(FMassUnitEntityHandle Entity, bool bSleeping, float CurrentTime) const;
	void ResetCrowdFragment(FMassUnitEntityHandle Entity, bool bStopUnit) const;
	void RemoveUnitFromPreviousGroup(FMassUnitEntityHandle Entity);
	FRandomStream MakeRandomStream(FMassUnitEntityHandle Entity, const FCrowdGroup& Group, int32 DecisionSequence) const;
	int32 CalculateSimulationLOD(const FVector& Location, const TArray<FVector>& ObserverLocations, float& OutDistanceSquared) const;
	float GetSimulationIntervalMultiplier(int32 LODLevel) const;
	FIntPoint ToSpatialCell(const FVector& Location) const;
	static FMassUnitCrowdConfig SanitizeConfig(const FMassUnitCrowdConfig& Config);
	bool IsEntityValid(FMassUnitEntityHandle Entity) const;
};
