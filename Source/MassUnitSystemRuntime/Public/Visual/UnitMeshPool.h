// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassUnitEntityManager.h"
#include "UnitMeshPool.generated.h"

class UMassEntitySubsystem;
class USkeletalMeshComponent;

/** Bounded pool for close-range skeletal representations. */
UCLASS(BlueprintType)
class MASSUNITSYSTEMRUNTIME_API UUnitMeshPool : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem, int32 PoolSize);
	void Deinitialize();
	void UpdateUnitMeshes(const TArray<FMassUnitEntityHandle>& Entities);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Rendering")
	USkeletalMeshComponent* GetMeshForUnit(FMassUnitHandle UnitHandle);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Rendering")
	void ReleaseMesh(USkeletalMeshComponent* Mesh);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Rendering")
	bool TransitionToSkeletal(FMassUnitHandle UnitHandle);

	UFUNCTION(BlueprintCallable, Category = "Mass Unit System|Rendering")
	bool TransitionToVertex(FMassUnitHandle UnitHandle);

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering|Diagnostics")
	int32 GetActiveSkeletalMeshCount() const { return EntityMeshMap.Num(); }

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering|Diagnostics")
	int32 GetAvailableSkeletalMeshCount() const { return AvailableMeshes.Num(); }

	UFUNCTION(BlueprintPure, Category = "Mass Unit System|Rendering|Diagnostics")
	int32 GetSkeletalMeshCapacity() const { return MaxPoolSize; }

	USkeletalMeshComponent* GetMeshForUnitInternal(FMassUnitEntityHandle Entity);
	bool TransitionToSkeletalInternal(FMassUnitEntityHandle Entity);
	bool TransitionToVertexInternal(FMassUnitEntityHandle Entity);

private:
	UPROPERTY(Transient)
	TObjectPtr<UWorld> World = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMassEntitySubsystem> EntitySubsystem = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USkeletalMeshComponent>> AvailableMeshes;

	UPROPERTY(Transient)
	TMap<FMassUnitEntityHandle, TObjectPtr<USkeletalMeshComponent>> EntityMeshMap;

	UPROPERTY(Transient)
	TMap<TObjectPtr<USkeletalMeshComponent>, FMassUnitEntityHandle> MeshEntityMap;

	TMap<TObjectPtr<USkeletalMeshComponent>, FGameplayTag> MeshAnimationTags;

	int32 MaxPoolSize = 100;
	float UpdateInterval = 0.033f;
	float LastUpdateTime = -BIG_NUMBER;

	USkeletalMeshComponent* CreateMeshComponent();
	void UpdateMeshFromEntity(USkeletalMeshComponent* Mesh, FMassUnitEntityHandle Entity);
	class UAnimationAsset* ResolveAnimationAsset(const struct FMassUnitVisualFragment& Visual) const;
	static bool ShouldLoopAnimation(const FGameplayTag& AnimationTag);
	bool IsEntityValid(FMassUnitEntityHandle Entity) const;
};
