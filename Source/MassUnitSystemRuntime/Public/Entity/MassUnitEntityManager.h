// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassEntityFallback.h"
#include "GameplayTagContainer.h"

#include "MassUnitEntityManager.generated.h"

class UUnitTemplate;

/**
 * Blueprint-friendly wrapper for FMassUnitEntityHandle
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitHandle
{
	GENERATED_BODY()

	/** The underlying Mass Entity handle */
FMassUnitEntityHandle EntityHandle;

	/** Default constructor */
	FMassUnitHandle() {}

	/** Constructor from FMassUnitEntityHandle */
	FMassUnitHandle(const FMassUnitEntityHandle& InEntityHandle) : EntityHandle(InEntityHandle) {}

	/** Conversion to FMassUnitEntityHandle */
	operator FMassUnitEntityHandle() const { return EntityHandle; }

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
	void Initialize(UMassUnitEntitySubsystem* InEntitySubsystem);

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

	/** Internal method to create a unit from a template (returns raw FMassUnitEntityHandle) */
	FMassUnitEntityHandle CreateUnitFromTemplateInternal(UUnitTemplate* Template, const FTransform& SpawnTransform);

	/** Internal method to destroy a unit (takes raw FMassUnitEntityHandle) */
	void DestroyUnitInternal(FMassUnitEntityHandle EntityHandle);

	/** Internal method to get units by type (returns raw FMassUnitEntityHandles) */
	TArray<FMassUnitEntityHandle> GetUnitsByTypeInternal(FGameplayTag UnitType);

	/** Internal method to get units by team (returns raw FMassUnitEntityHandles) */
	TArray<FMassUnitEntityHandle> GetUnitsByTeamInternal(int32 TeamID);

	/** Get the entity subsystem */
	UMassUnitEntitySubsystem* GetEntitySubsystem() const { return EntitySubsystem; }
	
	/** Get the unit type map */
	const TMap<FGameplayTag, TArray<FMassUnitEntityHandle>>& GetUnitTypeMap() const { return UnitTypeMap; }
	
	/** Get the team map */
	const TMap<int32, TArray<FMassUnitEntityHandle>>& GetTeamMap() const { return TeamMap; }

private:
	/** Reference to the Mass Entity Subsystem */
	UPROPERTY(Transient)
	UMassUnitEntitySubsystem* EntitySubsystem;

	/** Map of unit types to entity handles */
	TMap<FGameplayTag, TArray<FMassUnitEntityHandle>> UnitTypeMap;

	/** Map of team IDs to entity handles */
	TMap<int32, TArray<FMassUnitEntityHandle>> TeamMap;
};
