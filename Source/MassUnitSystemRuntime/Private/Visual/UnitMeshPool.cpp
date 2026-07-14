// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Visual/UnitMeshPool.h"

#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Config/MassUnitSystemSettings.h"
#include "Core/MassUnitGameplayTags.h"
#include "Engine/World.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"

void UUnitMeshPool::Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem, int32 PoolSize)
{
	World = InWorld;
	EntitySubsystem = InEntitySubsystem;
	MaxPoolSize = FMath::Max(0, PoolSize);
	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	UpdateInterval = Settings ? FMath::Max(0.0f, Settings->VisualUpdateInterval) : 0.033f;
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
	MeshAnimationTags.Reset();
	EntitySubsystem = nullptr;
	World = nullptr;
}

void UUnitMeshPool::UpdateUnitMeshes(const TArray<FMassUnitEntityHandle>& Entities)
{
	if (!World || World->GetTimeSeconds() - LastUpdateTime < UpdateInterval)
	{
		return;
	}
	LastUpdateTime = World->GetTimeSeconds();

	TArray<USkeletalMeshComponent*> MeshesToRelease;
	for (const TPair<FMassUnitEntityHandle, TObjectPtr<USkeletalMeshComponent>>& Pair : EntityMeshMap)
	{
		if (!IsEntityValid(Pair.Key))
		{
			MeshesToRelease.Add(Pair.Value);
			continue;
		}
		const FMassUnitVisualFragment* Visual = EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FMassUnitVisualFragment>(
			Pair.Key.ToMassEntityHandle());
		if (!Visual || !Visual->bWantsSkeletalMesh || !Visual->bIsVisible || !Visual->SkeletalMesh)
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
	struct FCandidate
	{
		FMassUnitEntityHandle Entity;
		float DistanceSquared = 0.0f;
	};
	TArray<FCandidate> Candidates;
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
		if (USkeletalMeshComponent* Existing = EntityMeshMap.FindRef(Entity))
		{
			Visual->bUseSkeletalMesh = true;
			UpdateMeshFromEntity(Existing, Entity);
		}
		else if (Visual->bWantsSkeletalMesh && Visual->bIsVisible && Visual->SkeletalMesh)
		{
			Candidates.Add({Entity, Visual->ViewerDistanceSquared});
			Visual->bUseSkeletalMesh = false;
		}
		else
		{
			Visual->bUseSkeletalMesh = false;
		}
	}

	Candidates.Sort([](const FCandidate& A, const FCandidate& B)
	{
		return A.DistanceSquared < B.DistanceSquared;
	});
	for (const FCandidate& Candidate : Candidates)
	{
		if (EntityMeshMap.Num() >= MaxPoolSize)
		{
			break;
		}
		GetMeshForUnitInternal(Candidate.Entity);
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
	if (FMassUnitVisualFragment* Visual = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitVisualFragment>(Entity.ToMassEntityHandle()))
	{
		Visual->bUseSkeletalMesh = true;
	}
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
	if (IsEntityValid(Entity))
	{
		if (FMassUnitVisualFragment* Visual = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitVisualFragment>(Entity.ToMassEntityHandle()))
		{
			Visual->bUseSkeletalMesh = false;
		}
	}
	MeshAnimationTags.Remove(Mesh);
	Mesh->SetVisibility(false);
	Mesh->SetAnimInstanceClass(nullptr);
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
	Visual->bWantsSkeletalMesh = true;
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
		Visual->bWantsSkeletalMesh = false;
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
		MeshAnimationTags.Remove(Mesh);
	}
	if (UAnimationAsset* Animation = ResolveAnimationAsset(*Visual))
	{
		if (MeshAnimationTags.FindRef(Mesh) != Visual->CurrentAnimation)
		{
			Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			Mesh->PlayAnimation(Animation, ShouldLoopAnimation(Visual->CurrentAnimation));
			MeshAnimationTags.Add(Mesh, Visual->CurrentAnimation);
		}
	}
	else if (Visual->AnimationBlueprintClass)
	{
		if (Mesh->GetAnimationMode() != EAnimationMode::AnimationBlueprint
			|| Mesh->GetAnimClass() != Visual->AnimationBlueprintClass.Get())
		{
			Mesh->SetAnimInstanceClass(Visual->AnimationBlueprintClass.Get());
			MeshAnimationTags.Remove(Mesh);
		}
	}
	Mesh->SetWorldTransform(Transform->GetTransform());
	Mesh->SetVisibility(Visual->bIsVisible, true);
}

UAnimationAsset* UUnitMeshPool::ResolveAnimationAsset(const FMassUnitVisualFragment& Visual) const
{
	using namespace UE::MassUnitSystem;
	if (Visual.CurrentAnimation == Tags::AnimationAttack())
	{
		return Visual.AttackAnimation;
	}
	if (Visual.CurrentAnimation == Tags::AnimationDeath())
	{
		return Visual.DeathAnimation;
	}
	if (Visual.CurrentAnimation == Tags::AnimationStun())
	{
		return Visual.StunAnimation;
	}
	if (Visual.CurrentAnimation == Tags::AnimationWalk() || Visual.CurrentAnimation == Tags::AnimationRun())
	{
		return Visual.MoveAnimation;
	}
	return Visual.IdleAnimation;
}

bool UUnitMeshPool::ShouldLoopAnimation(const FGameplayTag& AnimationTag)
{
	using namespace UE::MassUnitSystem;
	return AnimationTag != Tags::AnimationAttack() && AnimationTag != Tags::AnimationDeath();
}

bool UUnitMeshPool::IsEntityValid(FMassUnitEntityHandle Entity) const
{
	return EntitySubsystem && Entity.IsValid()
		&& EntitySubsystem->GetEntityManager().IsEntityValid(Entity.ToMassEntityHandle());
}
