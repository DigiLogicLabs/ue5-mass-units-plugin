// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Visual/NiagaraUnitSystem.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
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
	for (const TPair<TObjectPtr<UStaticMesh>, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>>& Pair : InstancedMeshComponents)
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
		AnimationIndices.Add(static_cast<float>(State->CurrentState));
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
		if (!Transform || !Visual || !Visual->bIsVisible || Visual->bUseSkeletalMesh)
		{
			continue;
		}
		UStaticMesh* Mesh = Visual->StaticMesh ? Visual->StaticMesh.Get() : FallbackStaticMesh.Get();
		if (Mesh)
		{
			InstancesByMesh.FindOrAdd(Mesh).Add(Transform->GetTransform());
		}
	}

	for (const TPair<TObjectPtr<UStaticMesh>, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>>& Pair : InstancedMeshComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->ClearInstances();
			Pair.Value->SetVisibility(false);
		}
	}
	for (const TPair<UStaticMesh*, TArray<FTransform>>& Pair : InstancesByMesh)
	{
		UHierarchicalInstancedStaticMeshComponent* Component = GetOrCreateInstancedMeshComponent(Pair.Key);
		if (!Component)
		{
			continue;
		}
		Component->SetVisibility(true);
		for (const FTransform& Transform : Pair.Value)
		{
			Component->AddInstance(Transform, true);
		}
	}
}

UHierarchicalInstancedStaticMeshComponent* UNiagaraUnitSystem::GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh)
{
	if (!Mesh || !World)
	{
		return nullptr;
	}
	if (UHierarchicalInstancedStaticMeshComponent* Existing = InstancedMeshComponents.FindRef(Mesh))
	{
		return Existing;
	}
	UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(World);
	if (!Component)
	{
		return nullptr;
	}
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetGenerateOverlapEvents(false);
	Component->SetStaticMesh(Mesh);
	Component->RegisterComponentWithWorld(World);
	InstancedMeshComponents.Add(Mesh, Component);
	return Component;
}
