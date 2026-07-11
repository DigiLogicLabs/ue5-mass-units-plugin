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

	int32 MaxPoolSize = 100;

	USkeletalMeshComponent* CreateMeshComponent();
	void UpdateMeshFromEntity(USkeletalMeshComponent* Mesh, FMassUnitEntityHandle Entity);
	bool IsEntityValid(FMassUnitEntityHandle Entity) const;
};
