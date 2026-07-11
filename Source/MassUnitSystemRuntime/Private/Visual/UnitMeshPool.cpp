// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Visual/UnitMeshPool.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"

void UUnitMeshPool::Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem, int32 PoolSize)
{
	World = InWorld;
	EntitySubsystem = InEntitySubsystem;
	MaxPoolSize = FMath::Max(0, PoolSize);
	const int32 PreCreateCount = FMath::Min(MaxPoolSize, 25);
	for (int32 Index = 0; Index < PreCreateCount; ++Index)
	{
		if (USkeletalMeshComponent* Component = CreateMeshComponent())
		{
			AvailableMeshes.Add(Component);
		}
	}
}

void UUnitMeshPool::Deinitialize()
{
	TSet<USkeletalMeshComponent*> Components;
	for (const TPair<FMassUnitEntityHandle, TObjectPtr<USkeletalMeshComponent>>& Pair : EntityMeshMap)
	{
		Components.Add(Pair.Value);
	}
	for (USkeletalMeshComponent* Component : AvailableMeshes)
	{
		Components.Add(Component);
	}
	for (USkeletalMeshComponent* Component : Components)
	{
		if (Component)
		{
			Component->DestroyComponent();
		}
	}
	AvailableMeshes.Reset();
	EntityMeshMap.Reset();
	MeshEntityMap.Reset();
	EntitySubsystem = nullptr;
	World = nullptr;
}

void UUnitMeshPool::UpdateUnitMeshes(const TArray<FMassUnitEntityHandle>& Entities)
{
	TArray<USkeletalMeshComponent*> MeshesToRelease;
	for (const TPair<FMassUnitEntityHandle, TObjectPtr<USkeletalMeshComponent>>& Pair : EntityMeshMap)
	{
		if (!IsEntityValid(Pair.Key) || !Entities.Contains(Pair.Key))
		{
			MeshesToRelease.Add(Pair.Value);
		}
	}
	for (USkeletalMeshComponent* Mesh : MeshesToRelease)
	{
		ReleaseMesh(Mesh);
	}

	if (!EntitySubsystem)
	{
		return;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	for (const FMassUnitEntityHandle Entity : Entities)
	{
		if (!IsEntityValid(Entity))
		{
			continue;
		}
		FMassUnitVisualFragment* Visual = EntityManager.GetFragmentDataPtr<FMassUnitVisualFragment>(Entity.ToMassEntityHandle());
		if (!Visual)
		{
			continue;
		}
		if (Visual->bUseSkeletalMesh && Visual->SkeletalMesh)
		{
			USkeletalMeshComponent* Mesh = GetMeshForUnitInternal(Entity);
			if (Mesh)
			{
				UpdateMeshFromEntity(Mesh, Entity);
			}
			else
			{
				Visual->bUseSkeletalMesh = false;
			}
		}
		else if (USkeletalMeshComponent* Existing = EntityMeshMap.FindRef(Entity))
		{
			ReleaseMesh(Existing);
		}
	}
}

USkeletalMeshComponent* UUnitMeshPool::GetMeshForUnit(FMassUnitHandle UnitHandle)
{
	return GetMeshForUnitInternal(UnitHandle.EntityHandle);
}

USkeletalMeshComponent* UUnitMeshPool::GetMeshForUnitInternal(FMassUnitEntityHandle Entity)
{
	if (!IsEntityValid(Entity) || MaxPoolSize <= 0)
	{
		return nullptr;
	}
	if (USkeletalMeshComponent* Existing = EntityMeshMap.FindRef(Entity))
	{
		return Existing;
	}
	if (EntityMeshMap.Num() >= MaxPoolSize)
	{
		return nullptr;
	}

	USkeletalMeshComponent* Mesh = nullptr;
	if (AvailableMeshes.IsEmpty())
	{
		Mesh = CreateMeshComponent();
	}
	else
	{
		Mesh = AvailableMeshes.Pop(EAllowShrinking::No).Get();
	}
	if (!Mesh)
	{
		return nullptr;
	}
	EntityMeshMap.Add(Entity, Mesh);
	MeshEntityMap.Add(Mesh, Entity);
	UpdateMeshFromEntity(Mesh, Entity);
	return Mesh;
}

void UUnitMeshPool::ReleaseMesh(USkeletalMeshComponent* Mesh)
{
	if (!Mesh)
	{
		return;
	}
	FMassUnitEntityHandle Entity;
	if (!MeshEntityMap.RemoveAndCopyValue(Mesh, Entity))
	{
		return;
	}
	EntityMeshMap.Remove(Entity);
	Mesh->SetVisibility(false);
	Mesh->SetSkeletalMeshAsset(nullptr);
	AvailableMeshes.AddUnique(Mesh);
}

bool UUnitMeshPool::TransitionToSkeletal(FMassUnitHandle UnitHandle)
{
	return TransitionToSkeletalInternal(UnitHandle.EntityHandle);
}

bool UUnitMeshPool::TransitionToSkeletalInternal(FMassUnitEntityHandle Entity)
{
	if (!IsEntityValid(Entity))
	{
		return false;
	}
	FMassUnitVisualFragment* Visual = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitVisualFragment>(Entity.ToMassEntityHandle());
	if (!Visual || !Visual->SkeletalMesh)
	{
		return false;
	}
	Visual->bUseSkeletalMesh = true;
	if (USkeletalMeshComponent* Mesh = GetMeshForUnitInternal(Entity))
	{
		Mesh->SetVisibility(true);
		return true;
	}
	Visual->bUseSkeletalMesh = false;
	return false;
}

bool UUnitMeshPool::TransitionToVertex(FMassUnitHandle UnitHandle)
{
	return TransitionToVertexInternal(UnitHandle.EntityHandle);
}

bool UUnitMeshPool::TransitionToVertexInternal(FMassUnitEntityHandle Entity)
{
	if (!IsEntityValid(Entity))
	{
		return false;
	}
	if (FMassUnitVisualFragment* Visual = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitVisualFragment>(Entity.ToMassEntityHandle()))
	{
		Visual->bUseSkeletalMesh = false;
	}
	if (USkeletalMeshComponent* Mesh = EntityMeshMap.FindRef(Entity))
	{
		ReleaseMesh(Mesh);
	}
	return true;
}

USkeletalMeshComponent* UUnitMeshPool::CreateMeshComponent()
{
	if (!World)
	{
		return nullptr;
	}
	USkeletalMeshComponent* Component = NewObject<USkeletalMeshComponent>(World);
	if (!Component)
	{
		return nullptr;
	}
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetGenerateOverlapEvents(false);
	Component->SetVisibility(false);
	Component->RegisterComponentWithWorld(World);
	return Component;
}

void UUnitMeshPool::UpdateMeshFromEntity(USkeletalMeshComponent* Mesh, FMassUnitEntityHandle Entity)
{
	if (!Mesh || !IsEntityValid(Entity))
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	const FMassEntityHandle NativeHandle = Entity.ToMassEntityHandle();
	const FMassUnitTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FMassUnitTransformFragment>(NativeHandle);
	const FMassUnitVisualFragment* Visual = EntityManager.GetFragmentDataPtr<FMassUnitVisualFragment>(NativeHandle);
	if (!Transform || !Visual || !Visual->SkeletalMesh)
	{
		return;
	}
	if (Mesh->GetSkeletalMeshAsset() != Visual->SkeletalMesh)
	{
		Mesh->SetSkeletalMeshAsset(Visual->SkeletalMesh);
	}
	Mesh->SetWorldTransform(Transform->GetTransform());
	Mesh->SetVisibility(Visual->bIsVisible, true);
}

bool UUnitMeshPool::IsEntityValid(FMassUnitEntityHandle Entity) const
{
	return EntitySubsystem && Entity.IsValid()
		&& EntitySubsystem->GetEntityManager().IsEntityValid(Entity.ToMassEntityHandle());
}
