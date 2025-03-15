// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassUnitEntityManager.generated.h"

class UUnitTemplate;
class UMassEntitySubsystem;

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
	FMassEntityHandle CreateUnitFromTemplate(UUnitTemplate* Template, const FTransform& SpawnTransform);

	/** Destroy a unit */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	void DestroyUnit(FMassEntityHandle EntityHandle);

	/** Get units by type */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	TArray<FMassEntityHandle> GetUnitsByType(FGameplayTag UnitType);

	/** Get units by team */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	TArray<FMassEntityHandle> GetUnitsByTeam(int32 TeamID);

	/** Get the entity subsystem */
	UMassEntitySubsystem* GetEntitySubsystem() const { return EntitySubsystem; }

private:
	/** Reference to the Mass Entity Subsystem */
	UPROPERTY(Transient)
	UMassEntitySubsystem* EntitySubsystem;

	/** Map of unit types to entity handles */
	TMap<FGameplayTag, TArray<FMassEntityHandle>> UnitTypeMap;

	/** Map of team IDs to entity handles */
	TMap<int32, TArray<FMassEntityHandle>> TeamMap;
};
