// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Visual/NiagaraUnitSystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Config/MassUnitSystemSettings.h"
#include "Core/MassUnitSystemRuntime.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassUnitCommonFragments.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Visual/VertexAnimationManager.h"

void UNiagaraUnitSystem::Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem)
{
	World = InWorld;
	EntitySubsystem = InEntitySubsystem;
	InstancedMeshTopologyRevision = 0;
	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	if (Settings)
	{
		MaxUnits = FMath::Max(1, Settings->MaxUnits);
		UpdateFrequency = FMath::Max(0.0f, Settings->VisualUpdateInterval);
		bEnableInstancedFallback = Settings->bEnableInstancedMeshFallback;
		NiagaraSystemAsset = Settings->DefaultNiagaraSystem.LoadSynchronous();
		FallbackStaticMesh = Settings->FallbackStaticMesh.LoadSynchronous();
	}
	if (!FallbackStaticMesh && bEnableInstancedFallback)
	{
		FallbackStaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}
	CreateNiagaraSystem();
	VertexAnimationManager = NewObject<UVertexAnimationManager>(this);
	VertexAnimationManager->Initialize();
}

void UNiagaraUnitSystem::Deinitialize()
{
	if (NiagaraComponent)
	{
		NiagaraComponent->DestroyComponent();
	}
	for (const TPair<TObjectPtr<UStaticMesh>, TObjectPtr<UInstancedStaticMeshComponent>>& Pair : InstancedMeshComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->DestroyComponent();
		}
	}
	if (VertexAnimationManager)
	{
		VertexAnimationManager->Deinitialize();
	}
	InstancedMeshComponents.Reset();
	InstancedMeshTopologyRevision = 0;
	VertexAnimationManager = nullptr;
	NiagaraComponent = nullptr;
	NiagaraSystemAsset = nullptr;
	FallbackStaticMesh = nullptr;
	EntitySubsystem = nullptr;
	World = nullptr;
}

void UNiagaraUnitSystem::UpdateUnitVisualsByHandles(const TArray<FMassUnitHandle>& UnitHandles)
{
	TArray<FMassUnitEntityHandle> Entities;
	Entities.Reserve(UnitHandles.Num());
	for (const FMassUnitHandle& Handle : UnitHandles)
	{
		Entities.Add(Handle.EntityHandle);
	}
	UpdateUnitVisuals(Entities);
}

void UNiagaraUnitSystem::UpdateUnitVisuals(const TArray<FMassUnitEntityHandle>& Entities)
{
	if (!World || !EntitySubsystem)
	{
		return;
	}
	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastUpdateTime < UpdateFrequency)
	{
		return;
	}
	LastUpdateTime = CurrentTime;
	if (NiagaraComponent)
	{
		UpdateNiagaraData(Entities);
	}
	else if (bEnableInstancedFallback)
	{
		UpdateInstancedMeshData(Entities);
	}
}

void UNiagaraUnitSystem::SetLODLevel(int32 LODLevel)
{
	CurrentLODLevel = FMath::Max(0, LODLevel);
	if (NiagaraComponent)
	{
		NiagaraComponent->SetIntParameter(TEXT("LODLevel"), CurrentLODLevel);
	}
}

int32 UNiagaraUnitSystem::GetInstancedMeshInstanceCount() const
{
	int32 InstanceCount = 0;
	for (const TPair<TObjectPtr<UStaticMesh>, TObjectPtr<UInstancedStaticMeshComponent>>& Pair : InstancedMeshComponents)
	{
		if (Pair.Value)
		{
			InstanceCount += Pair.Value->GetInstanceCount();
		}
	}
	return InstanceCount;
}

void UNiagaraUnitSystem::CreateNiagaraSystem()
{
	if (!World || !NiagaraSystemAsset)
	{
		if (!NiagaraSystemAsset && bEnableInstancedFallback)
		{
			UE_LOG(LogMassUnitSystem, Log, TEXT("No default Niagara asset configured; using instanced static mesh rendering"));
		}
		return;
	}
	NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		NiagaraSystemAsset,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector,
		true,
		false,
		ENCPoolMethod::None,
		true);
	if (NiagaraComponent)
	{
		NiagaraComponent->SetIntParameter(TEXT("MaxUnits"), MaxUnits);
		NiagaraComponent->SetIntParameter(TEXT("LODLevel"), CurrentLODLevel);
	}
}

void UNiagaraUnitSystem::UpdateNiagaraData(const TArray<FMassUnitEntityHandle>& Entities)
{
	if (!NiagaraComponent || !EntitySubsystem)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TArray<FVector> Positions;
	TArray<FVector> Velocities;
	TArray<FVector> Scales;
	TArray<FQuat> Rotations;
	TArray<FVector> TeamColors;
	TArray<float> TeamIDs;
	TArray<float> AnimationIndices;
	TArray<float> AnimationTimes;
	TArray<float> LODLevels;
	TArray<float> VisibilityFlags;
	const int32 Count = FMath::Min(MaxUnits, Entities.Num());
	Positions.Reserve(Count);
	Velocities.Reserve(Count);
	Scales.Reserve(Count);
	Rotations.Reserve(Count);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FMassEntityHandle NativeHandle = Entities[Index].ToMassEntityHandle();
		if (!EntityManager.IsEntityValid(NativeHandle))
		{
			continue;
		}
		const FMassUnitTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FMassUnitTransformFragment>(NativeHandle);
		const FMassUnitVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassUnitVelocityFragment>(NativeHandle);
		const FMassUnitVisualFragment* Visual = EntityManager.GetFragmentDataPtr<FMassUnitVisualFragment>(NativeHandle);
		const FMassUnitTeamFragment* Team = EntityManager.GetFragmentDataPtr<FMassUnitTeamFragment>(NativeHandle);
		const FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(NativeHandle);
		if (!Transform || !Velocity || !Visual || !Team || !State || !Visual->bIsVisible || Visual->bUseSkeletalMesh)
		{
			continue;
		}

		const FTransform& UnitTransform = Transform->GetTransform();
		Positions.Add(UnitTransform.GetLocation());
		Velocities.Add(Velocity->Value);
		Scales.Add(UnitTransform.GetScale3D());
		Rotations.Add(UnitTransform.GetRotation());
		TeamColors.Add(FVector(Team->TeamColor.R, Team->TeamColor.G, Team->TeamColor.B));
		TeamIDs.Add(static_cast<float>(Team->TeamID));
		AnimationIndices.Add(static_cast<float>(ResolveAnimationIndex(*Visual, *State)));
		AnimationTimes.Add(State->StateTime);
		LODLevels.Add(static_cast<float>(Visual->LODLevel));
		VisibilityFlags.Add(1.0f);
	}

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, TEXT("UnitPositions"), Positions);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, TEXT("UnitVelocities"), Velocities);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, TEXT("UnitScales"), Scales);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(NiagaraComponent, TEXT("UnitRotations"), Rotations);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, TEXT("UnitTeamColors"), TeamColors);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComponent, TEXT("UnitTeamIDs"), TeamIDs);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComponent, TEXT("UnitAnimationIndices"), AnimationIndices);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComponent, TEXT("UnitAnimationTimes"), AnimationTimes);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComponent, TEXT("UnitLODLevels"), LODLevels);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComponent, TEXT("UnitVisibilityFlags"), VisibilityFlags);
	NiagaraComponent->SetIntParameter(TEXT("UnitCount"), Positions.Num());
}

void UNiagaraUnitSystem::UpdateInstancedMeshData(const TArray<FMassUnitEntityHandle>& Entities)
{
	if (!EntitySubsystem)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TMap<UStaticMesh*, TArray<FTransform>> InstancesByMesh;
	TMap<UStaticMesh*, TArray<float>> CustomDataByMesh;
	const int32 Count = FMath::Min(MaxUnits, Entities.Num());
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FMassEntityHandle NativeHandle = Entities[Index].ToMassEntityHandle();
		if (!EntityManager.IsEntityValid(NativeHandle))
		{
			continue;
		}
		const FMassUnitTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FMassUnitTransformFragment>(NativeHandle);
		const FMassUnitVisualFragment* Visual = EntityManager.GetFragmentDataPtr<FMassUnitVisualFragment>(NativeHandle);
		const FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(NativeHandle);
		const FMassUnitTeamFragment* Team = EntityManager.GetFragmentDataPtr<FMassUnitTeamFragment>(NativeHandle);
		if (!Transform || !Visual || !State || !Team || !Visual->bIsVisible || Visual->bUseSkeletalMesh)
		{
			continue;
		}
		UStaticMesh* Mesh = Visual->StaticMesh ? Visual->StaticMesh.Get() : FallbackStaticMesh.Get();
		if (Mesh)
		{
			InstancesByMesh.FindOrAdd(Mesh).Add(Transform->GetTransform());
			TArray<float>& CustomData = CustomDataByMesh.FindOrAdd(Mesh);
			CustomData.Reserve(InstancesByMesh[Mesh].Num() * InstancedCustomDataFloatCount);
			CustomData.Add(static_cast<float>(ResolveAnimationIndex(*Visual, *State)));
			CustomData.Add(State->StateTime);
			CustomData.Add(static_cast<float>(Visual->LODLevel));
			CustomData.Add(static_cast<float>(Team->TeamID));
			CustomData.Add(Team->TeamColor.R);
			CustomData.Add(Team->TeamColor.G);
			CustomData.Add(Team->TeamColor.B);
			CustomData.Add(State->MaxHealth > UE_SMALL_NUMBER ? State->Health / State->MaxHealth : 0.0f);
		}
	}

	static const TArray<FTransform> EmptyTransforms;
	for (const TPair<TObjectPtr<UStaticMesh>, TObjectPtr<UInstancedStaticMeshComponent>>& Pair : InstancedMeshComponents)
	{
		const TArray<FTransform>* DesiredTransforms = InstancesByMesh.Find(Pair.Key.Get());
		const TArray<float>* DesiredCustomData = CustomDataByMesh.Find(Pair.Key.Get());
		static const TArray<float> EmptyCustomData;
		SynchronizeInstancedMeshComponent(
			Pair.Value,
			DesiredTransforms ? *DesiredTransforms : EmptyTransforms,
			DesiredCustomData ? *DesiredCustomData : EmptyCustomData);
	}
	for (const TPair<UStaticMesh*, TArray<FTransform>>& Pair : InstancesByMesh)
	{
		UInstancedStaticMeshComponent* Component = GetOrCreateInstancedMeshComponent(Pair.Key);
		if (!Component)
		{
			continue;
		}
		if (Component->GetInstanceCount() == 0)
		{
			SynchronizeInstancedMeshComponent(Component, Pair.Value, CustomDataByMesh.FindChecked(Pair.Key));
		}
	}
}

void UNiagaraUnitSystem::SynchronizeInstancedMeshComponent(
	UInstancedStaticMeshComponent* Component,
	const TArray<FTransform>& WorldTransforms,
	const TArray<float>& CustomData)
{
	if (!Component)
	{
		return;
	}

	const int32 ExistingCount = Component->GetInstanceCount();
	const int32 DesiredCount = WorldTransforms.Num();
	const int32 SharedCount = FMath::Min(ExistingCount, DesiredCount);
	if (SharedCount > 0)
	{
		Component->BatchUpdateInstancesTransforms(
			0,
			MakeArrayView(WorldTransforms).Left(SharedCount),
			true,
			false,
			false);
	}

	if (DesiredCount > ExistingCount)
	{
		const int32 AddedCount = DesiredCount - ExistingCount;
		Component->PreAllocateInstancesMemory(AddedCount);
		TArray<FTransform> AddedTransforms;
		AddedTransforms.Append(WorldTransforms.GetData() + ExistingCount, AddedCount);
		Component->AddInstances(AddedTransforms, false, true, false);
		++InstancedMeshTopologyRevision;
	}
	else if (ExistingCount > DesiredCount)
	{
		TArray<int32> RemovedIndices;
		RemovedIndices.Reserve(ExistingCount - DesiredCount);
		for (int32 InstanceIndex = ExistingCount - 1; InstanceIndex >= DesiredCount; --InstanceIndex)
		{
			RemovedIndices.Add(InstanceIndex);
		}
		Component->RemoveInstances(RemovedIndices, true);
		++InstancedMeshTopologyRevision;
	}

	if (DesiredCount > 0 && CustomData.Num() == DesiredCount * InstancedCustomDataFloatCount)
	{
		Component->SetCustomData(0, DesiredCount - 1, CustomData, true);
	}

	const bool bShouldBeVisible = DesiredCount > 0;
	if (Component->IsVisible() != bShouldBeVisible)
	{
		Component->SetVisibility(bShouldBeVisible);
	}
}

UInstancedStaticMeshComponent* UNiagaraUnitSystem::GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh)
{
	if (!Mesh || !World)
	{
		return nullptr;
	}
	if (UInstancedStaticMeshComponent* Existing = InstancedMeshComponents.FindRef(Mesh))
	{
		return Existing;
	}
	UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(World);
	if (!Component)
	{
		return nullptr;
	}
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetGenerateOverlapEvents(false);
	Component->SetCanEverAffectNavigation(false);
	Component->SetNumCustomDataFloats(InstancedCustomDataFloatCount);
	Component->SetStaticMesh(Mesh);
	Component->RegisterComponentWithWorld(World);
	InstancedMeshComponents.Add(Mesh, Component);
	return Component;
}

int32 UNiagaraUnitSystem::ResolveAnimationIndex(
	const FMassUnitVisualFragment& Visual,
	const FMassUnitStateFragment& State)
{
	if (!VertexAnimationManager)
	{
		return static_cast<int32>(State.CurrentState);
	}
	if (Visual.VertexAnimationTexture && Visual.CurrentAnimation.IsValid())
	{
		VertexAnimationManager->RegisterAnimationTexture(Visual.CurrentAnimation, Visual.VertexAnimationTexture);
	}
	const int32 AnimationIndex = VertexAnimationManager->GetAnimationIndex(Visual.CurrentAnimation);
	return AnimationIndex != INDEX_NONE ? AnimationIndex : static_cast<int32>(State.CurrentState);
}
