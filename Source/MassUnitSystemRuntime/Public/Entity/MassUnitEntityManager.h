// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassEntityFallback.h"
#include "Entity/MassUnitFragments.h"
#include "GameplayTagContainer.h"
#include "MassArchetypeTypes.h"
#include "MassEntityTypes.h"
#include "MassUnitEntityManager.generated.h"

class UMassEntitySubsystem;
class UUnitTemplate;
class AActor;

/** Blueprint-safe wrapper around a world-owned Mass entity handle. */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitHandle
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	FMassUnitEntityHandle EntityHandle;

	FMassUnitHandle() = default;
	explicit FMassUnitHandle(const FMassUnitEntityHandle& InEntityHandle) : EntityHandle(InEntityHandle) {}
	explicit FMassUnitHandle(const FMassEntityHandle InEntityHandle) : EntityHandle(InEntityHandle) {}

	operator FMassUnitEntityHandle() const { return EntityHandle; }
	bool IsValid() const { return EntityHandle.IsValid(); }
	friend bool operator==(const FMassUnitHandle& A, const FMassUnitHandle& B) { return A.EntityHandle == B.EntityHandle; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FMassUnitHealthChangedSignature,
	FMassUnitHandle, UnitHandle,
	float, PreviousHealth,
	float, NewHealth,
	AActor*, InstigatorActor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMassUnitDiedSignature,
	FMassUnitHandle, UnitHandle,
	AActor*, InstigatorActor);

/** World-scoped facade for creating and accessing plugin-owned Mass entities. */
UCLASS(BlueprintType)
class MASSUNITSYSTEMRUNTIME_API UMassUnitEntityManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UMassEntitySubsystem* InEntitySubsystem);
	void Deinitialize();

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	FMassUnitHandle CreateUnitFromTemplate(UUnitTemplate* Template, const FTransform& SpawnTransform);

	/** Creates a unit with the plugin's built-in gameplay defaults and asset-free cube representation. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System", meta = (DisplayName = "Create Default Unit", Keywords = "quick start spawn cube"))
	FMassUnitHandle CreateDefaultUnit(const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	void DestroyUnit(FMassUnitHandle UnitHandle);

	UFUNCTION(BlueprintPure, Category = "Mass Unit System")
	bool IsUnitValid(FMassUnitHandle UnitHandle) const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System")
	int32 GetUnitCount() const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System")
	TArray<FMassUnitHandle> GetAllUnits() const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System")
	TArray<FMassUnitHandle> GetUnitsByType(FGameplayTag UnitType) const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System")
	TArray<FMassUnitHandle> GetUnitsByTeam(int32 TeamID) const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System")
	bool GetUnitTransform(FMassUnitHandle UnitHandle, FTransform& OutTransform) const;

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	bool SetUnitTransform(FMassUnitHandle UnitHandle, const FTransform& NewTransform);

	/** Assigns a direct movement destination. Use the navigation system when navmesh routing is desired. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	bool SetUnitDestination(FMassUnitHandle UnitHandle, const FVector& Destination, float AcceptanceRadius = 50.0f);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	bool SetUnitTarget(FMassUnitHandle UnitHandle, FMassUnitHandle TargetUnit, float Priority = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
	bool ClearUnitTarget(FMassUnitHandle UnitHandle);

	UFUNCTION(BlueprintPure, Category = "Mass Unit System")
	bool GetUnitState(FMassUnitHandle UnitHandle, FMassUnitStateFragment& OutState) const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Health")
	bool GetUnitHealth(FMassUnitHandle UnitHandle, float& OutHealth, float& OutMaxHealth) const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Health")
	float GetUnitHealthPercent(FMassUnitHandle UnitHandle) const;

	/** Applies direct lightweight damage. GAS-backed damage can be routed through UGASUnitIntegration instead. */
	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Health")
	bool ApplyDamage(FMassUnitHandle UnitHandle, float Damage, AActor* DamageInstigator = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Health")
	bool HealUnit(FMassUnitHandle UnitHandle, float Amount, AActor* HealInstigator = nullptr);

	/** Returns the closest managed unit inside Max Distance. Choose planar or full XYZ distance per query. */
	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Queries")
	FMassUnitHandle FindClosestUnit(
		FVector WorldLocation,
		float MaxDistance,
		bool bUse3DDistance = false,
		bool bIncludeDead = false) const;

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Queries")
	TArray<FMassUnitHandle> GetUnitsInRadius(
		FVector WorldLocation,
		float Radius,
		bool bUse3DDistance = false,
		bool bIncludeDead = false) const;

	UPROPERTY(BlueprintAssignable, Category = "Mass Unit System|Health|Events")
	FMassUnitHealthChangedSignature OnUnitHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Mass Unit System|Health|Events")
	FMassUnitDiedSignature OnUnitDied;

	FMassUnitEntityHandle CreateUnitFromTemplateInternal(UUnitTemplate* Template, const FTransform& SpawnTransform);
	void DestroyUnitInternal(FMassUnitEntityHandle EntityHandle);
	const TArray<FMassUnitEntityHandle>& GetAllUnitsInternal() const { return AllUnits; }
	TArray<FMassUnitEntityHandle> GetUnitsByTypeInternal(FGameplayTag UnitType) const;
	TArray<FMassUnitEntityHandle> GetUnitsByTeamInternal(int32 TeamID) const;
	bool ApplyDamageInternal(
		FMassUnitEntityHandle EntityHandle,
		float Damage,
		AActor* DamageInstigator = nullptr,
		bool bDeferEvents = false);
	void PruneInvalidUnits();

	UMassEntitySubsystem* GetEntitySubsystem() const { return EntitySubsystem; }
	const TMap<FGameplayTag, TArray<FMassUnitEntityHandle>>& GetUnitTypeMap() const { return UnitTypeMap; }
	const TMap<int32, TArray<FMassUnitEntityHandle>>& GetTeamMap() const { return TeamMap; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UMassEntitySubsystem> EntitySubsystem = nullptr;

	/** Lazily created template used by CreateDefaultUnit. */
	UPROPERTY(Transient)
	TObjectPtr<UUnitTemplate> RuntimeDefaultTemplate = nullptr;

	FMassArchetypeHandle UnitArchetype;
	TArray<FMassUnitEntityHandle> AllUnits;
	TMap<FGameplayTag, TArray<FMassUnitEntityHandle>> UnitTypeMap;
	TMap<int32, TArray<FMassUnitEntityHandle>> TeamMap;

	void RemoveHandleFromIndexes(FMassUnitEntityHandle EntityHandle);
};
