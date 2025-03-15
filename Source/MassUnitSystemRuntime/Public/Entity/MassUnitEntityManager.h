// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassEntityFallback.h"

#include "MassUnitEntityManager.generated.h"

class UUnitTemplate;
class UMassEntitySubsystem;

/**
 * Blueprint-friendly wrapper for FMassEntityHandle
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitHandle
{
	GENERATED_BODY()

	/** The underlying Mass Entity handle */
	FMassEntityHandle EntityHandle;

	/** Default constructor */
	FMassUnitHandle() {}

	/** Constructor from FMassEntityHandle */
	FMassUnitHandle(const FMassEntityHandle& InEntityHandle) : EntityHandle(InEntityHandle) {}

	/** Conversion to FMassEntityHandle */
	operator FMassEntityHandle() const { return EntityHandle; }

	/** Check if the handle is valid */
	FORCEINLINE bool IsValid() const { return EntityHandle.IsValid(); }
};

/**
 * Central manager for all unit entities in the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UMassUnitEntityManager : public UObject
{
	GENERATED_BODY()

public:
	UMassUnitEntityManager();
	virtual ~UMassUnitEntityManager();

	/** Initialize the manager */
	void Initialize(UMassEntitySubsystem* InEntitySubsystem);

	/** Create a unit from a template */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	FMassUnitHandle CreateUnitFromTemplate(UUnitTemplate* Template, const FTransform& SpawnTransform);

	/** Destroy a unit */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	void DestroyUnit(FMassUnitHandle UnitHandle);

	/** Get units by type */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	TArray<FMassUnitHandle> GetUnitsByType(FGameplayTag UnitType);

	/** Get units by team */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	TArray<FMassUnitHandle> GetUnitsByTeam(int32 TeamID);

	/** Internal method to create a unit from a template (returns raw FMassEntityHandle) */
	FMassEntityHandle CreateUnitFromTemplateInternal(UUnitTemplate* Template, const FTransform& SpawnTransform);

	/** Internal method to destroy a unit (takes raw FMassEntityHandle) */
	void DestroyUnitInternal(FMassEntityHandle EntityHandle);

	/** Internal method to get units by type (returns raw FMassEntityHandles) */
	TArray<FMassEntityHandle> GetUnitsByTypeInternal(FGameplayTag UnitType);

	/** Internal method to get units by team (returns raw FMassEntityHandles) */
	TArray<FMassEntityHandle> GetUnitsByTeamInternal(int32 TeamID);

	/** Get the entity subsystem */
	UMassEntitySubsystem* GetEntitySubsystem() const { return EntitySubsystem; }
	
	/** Get the unit type map */
	const TMap<FGameplayTag, TArray<FMassEntityHandle>>& GetUnitTypeMap() const { return UnitTypeMap; }
	
	/** Get the team map */
	const TMap<int32, TArray<FMassEntityHandle>>& GetTeamMap() const { return TeamMap; }

private:
	/** Reference to the Mass Entity Subsystem */
	UPROPERTY(Transient)
	UMassEntitySubsystem* EntitySubsystem;

	/** Map of unit types to entity handles */
	TMap<FGameplayTag, TArray<FMassEntityHandle>> UnitTypeMap;

	/** Map of team IDs to entity handles */
	TMap<int32, TArray<FMassEntityHandle>> TeamMap;
};
