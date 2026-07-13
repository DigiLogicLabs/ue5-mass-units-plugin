// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitEntityManager.h"

#include "Config/MassUnitSystemSettings.h"
#include "Core/MassUnitGameplayTags.h"
#include "Core/MassUnitSystemRuntime.h"
#include "Entity/UnitTemplate.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassEntityView.h"
#include "MassUnitCommonFragments.h"

void UMassUnitEntityManager::Initialize(UMassEntitySubsystem* InEntitySubsystem)
{
	EntitySubsystem = InEntitySubsystem;
	UE_LOG(LogMassUnitSystem, Log, TEXT("Unit entity manager initialized"));
}

void UMassUnitEntityManager::Deinitialize()
{
	// The world-owned Mass entity manager destroys its entities during world teardown.
	// It can already be unavailable when dependent world subsystems are deinitialized,
	// so touching it here would be both redundant and unsafe.
	AllUnits.Reset();
	UnitTypeMap.Reset();
	TeamMap.Reset();
	UnitArchetype = FMassArchetypeHandle();
	RuntimeDefaultTemplate = nullptr;
	EntitySubsystem = nullptr;
}

FMassUnitHandle UMassUnitEntityManager::CreateUnitFromTemplate(UUnitTemplate* Template, const FTransform& SpawnTransform)
{
	return FMassUnitHandle(CreateUnitFromTemplateInternal(Template, SpawnTransform));
}

FMassUnitHandle UMassUnitEntityManager::CreateDefaultUnit(const FTransform& SpawnTransform)
{
	if (!RuntimeDefaultTemplate)
	{
		RuntimeDefaultTemplate = NewObject<UUnitTemplate>(this, TEXT("RuntimeDefaultUnitTemplate"), RF_Transient);
	}
	return CreateUnitFromTemplate(RuntimeDefaultTemplate, SpawnTransform);
}

FMassUnitEntityHandle UMassUnitEntityManager::CreateUnitFromTemplateInternal(UUnitTemplate* Template, const FTransform& SpawnTransform)
{
	if (!Template || !EntitySubsystem)
	{
		UE_LOG(LogMassUnitSystem, Warning, TEXT("CreateUnitFromTemplate rejected an invalid template or uninitialized manager"));
		return {};
	}

	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	if (Settings && AllUnits.Num() >= Settings->MaxUnits)
	{
		UE_LOG(LogMassUnitSystem, Warning, TEXT("Unit limit (%d) reached"), Settings->MaxUnits);
		return {};
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	if (!UnitArchetype.IsValid())
	{
		UnitArchetype = EntityManager.CreateArchetype(Template->GetRequiredFragments());
	}

	const FMassEntityHandle NativeHandle = EntityManager.CreateEntity(UnitArchetype);
	if (!EntityManager.IsEntityValid(NativeHandle))
	{
		UE_LOG(LogMassUnitSystem, Error, TEXT("Mass failed to create a unit entity"));
		return {};
	}

	FMassEntityView EntityView(EntityManager, NativeHandle);
	EntityView.GetFragmentData<FMassUnitTransformFragment>().SetTransform(SpawnTransform);

	FMassUnitStateFragment& State = EntityView.GetFragmentData<FMassUnitStateFragment>();
	State.CurrentState = EMassUnitState::Idle;
	State.UnitType = Template->UnitType.IsValid() ? Template->UnitType : UE::MassUnitSystem::Tags::UnitTypeDefault();
	State.UnitClass = Template->UnitClass;
	State.DefaultBehavior = Template->DefaultBehavior;
	State.UnitLevel = FMath::Max(1, Template->BaseLevel);
	State.MaxHealth = FMath::Max(0.0f, static_cast<float>(Template->BaseHealth));
	State.Health = State.MaxHealth;
	State.BaseDamage = FMath::Max(0.0f, static_cast<float>(Template->BaseDamage));
	State.MoveSpeed = FMath::Max(0.0f, Template->MoveSpeed);
	State.AttackRange = FMath::Max(0.0f, Template->AttackRange);
	State.AttackCooldown = FMath::Max(0.0f, Template->AttackCooldown);

	FMassUnitTeamFragment& Team = EntityView.GetFragmentData<FMassUnitTeamFragment>();
	Team.TeamID = Template->TeamID;
	Team.TeamColor = Template->TeamColor;
	Team.TeamFaction = Template->TeamFaction;

	FMassUnitAbilityFragment& Ability = EntityView.GetFragmentData<FMassUnitAbilityFragment>();
	Ability.DefaultAbilityTags = Template->DefaultAbilities;

	FMassUnitVisualFragment& Visual = EntityView.GetFragmentData<FMassUnitVisualFragment>();
	Visual.SkeletalMesh = Template->SkeletalMesh.LoadSynchronous();
	Visual.StaticMesh = Template->StaticMesh.LoadSynchronous();
	Visual.VertexAnimationTexture = Template->VertexAnimationTexture.LoadSynchronous();
	Visual.NormalMapTexture = Template->NormalMapTexture.LoadSynchronous();
	Visual.AnimationTags = Template->AnimationTags;
	Visual.CurrentAnimation = UE::MassUnitSystem::Tags::AnimationIdle();
	Visual.TargetAnimation = Visual.CurrentAnimation;
	Visual.bUseSkeletalMesh = false;
	Visual.bIsVisible = true;

	FMassUnitFormationFragment& Formation = EntityView.GetFragmentData<FMassUnitFormationFragment>();
	Formation.DefaultFormation = Template->DefaultFormation;

	const FMassUnitEntityHandle Handle(NativeHandle);
	AllUnits.Add(Handle);
	UnitTypeMap.FindOrAdd(State.UnitType).Add(Handle);
	TeamMap.FindOrAdd(Team.TeamID).Add(Handle);

	UE_LOG(LogMassUnitSystem, Verbose, TEXT("Created %s (%s, team %d)"), *Handle.ToString(), *State.UnitType.ToString(), Team.TeamID);
	return Handle;
}

void UMassUnitEntityManager::DestroyUnit(FMassUnitHandle UnitHandle)
{
	DestroyUnitInternal(UnitHandle.EntityHandle);
}

void UMassUnitEntityManager::DestroyUnitInternal(FMassUnitEntityHandle EntityHandle)
{
	if (!EntitySubsystem || !EntityHandle.IsValid())
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle NativeHandle = EntityHandle.ToMassEntityHandle();
	if (!EntityManager.IsEntityValid(NativeHandle))
	{
		RemoveHandleFromIndexes(EntityHandle);
		return;
	}

	const FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(NativeHandle);
	const FMassUnitTeamFragment* Team = EntityManager.GetFragmentDataPtr<FMassUnitTeamFragment>(NativeHandle);
	if (State)
	{
		if (TArray<FMassUnitEntityHandle>* Units = UnitTypeMap.Find(State->UnitType))
		{
			Units->Remove(EntityHandle);
			if (Units->IsEmpty())
			{
				UnitTypeMap.Remove(State->UnitType);
			}
		}
	}
	if (Team)
	{
		if (TArray<FMassUnitEntityHandle>* Units = TeamMap.Find(Team->TeamID))
		{
			Units->Remove(EntityHandle);
			if (Units->IsEmpty())
			{
				TeamMap.Remove(Team->TeamID);
			}
		}
	}

	AllUnits.Remove(EntityHandle);
	EntityManager.DestroyEntity(NativeHandle);
	UE_LOG(LogMassUnitSystem, Verbose, TEXT("Destroyed %s"), *EntityHandle.ToString());
}

bool UMassUnitEntityManager::IsUnitValid(FMassUnitHandle UnitHandle) const
{
	return EntitySubsystem && UnitHandle.IsValid()
		&& EntitySubsystem->GetEntityManager().IsEntityValid(UnitHandle.EntityHandle.ToMassEntityHandle());
}

int32 UMassUnitEntityManager::GetUnitCount() const
{
	if (!EntitySubsystem)
	{
		return 0;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	int32 ValidUnitCount = 0;
	for (const FMassUnitEntityHandle Handle : AllUnits)
	{
		ValidUnitCount += Handle.IsValid() && EntityManager.IsEntityValid(Handle.ToMassEntityHandle()) ? 1 : 0;
	}
	return ValidUnitCount;
}

TArray<FMassUnitHandle> UMassUnitEntityManager::GetAllUnits() const
{
	TArray<FMassUnitHandle> Result;
	Result.Reserve(AllUnits.Num());
	for (const FMassUnitEntityHandle Handle : AllUnits)
	{
		if (EntitySubsystem && EntitySubsystem->GetEntityManager().IsEntityValid(Handle.ToMassEntityHandle()))
		{
			Result.Emplace(Handle);
		}
	}
	return Result;
}

TArray<FMassUnitHandle> UMassUnitEntityManager::GetUnitsByType(FGameplayTag UnitType) const
{
	TArray<FMassUnitHandle> Result;
	for (const FMassUnitEntityHandle Handle : GetUnitsByTypeInternal(UnitType))
	{
		Result.Emplace(Handle);
	}
	return Result;
}

TArray<FMassUnitHandle> UMassUnitEntityManager::GetUnitsByTeam(int32 TeamID) const
{
	TArray<FMassUnitHandle> Result;
	for (const FMassUnitEntityHandle Handle : GetUnitsByTeamInternal(TeamID))
	{
		Result.Emplace(Handle);
	}
	return Result;
}

TArray<FMassUnitEntityHandle> UMassUnitEntityManager::GetUnitsByTypeInternal(FGameplayTag UnitType) const
{
	if (const TArray<FMassUnitEntityHandle>* Units = UnitTypeMap.Find(UnitType))
	{
		TArray<FMassUnitEntityHandle> Result = *Units;
		Result.RemoveAll([this](const FMassUnitEntityHandle Handle)
		{
			return !EntitySubsystem || !EntitySubsystem->GetEntityManager().IsEntityValid(Handle.ToMassEntityHandle());
		});
		return Result;
	}
	return {};
}

TArray<FMassUnitEntityHandle> UMassUnitEntityManager::GetUnitsByTeamInternal(int32 TeamID) const
{
	if (const TArray<FMassUnitEntityHandle>* Units = TeamMap.Find(TeamID))
	{
		TArray<FMassUnitEntityHandle> Result = *Units;
		Result.RemoveAll([this](const FMassUnitEntityHandle Handle)
		{
			return !EntitySubsystem || !EntitySubsystem->GetEntityManager().IsEntityValid(Handle.ToMassEntityHandle());
		});
		return Result;
	}
	return {};
}

bool UMassUnitEntityManager::GetUnitTransform(FMassUnitHandle UnitHandle, FTransform& OutTransform) const
{
	if (!IsUnitValid(UnitHandle))
	{
		return false;
	}
	const FMassUnitTransformFragment* Transform = EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FMassUnitTransformFragment>(UnitHandle.EntityHandle.ToMassEntityHandle());
	if (!Transform)
	{
		return false;
	}
	OutTransform = Transform->GetTransform();
	return true;
}

bool UMassUnitEntityManager::SetUnitTransform(FMassUnitHandle UnitHandle, const FTransform& NewTransform)
{
	if (!IsUnitValid(UnitHandle))
	{
		return false;
	}
	FMassUnitTransformFragment* Transform = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitTransformFragment>(UnitHandle.EntityHandle.ToMassEntityHandle());
	if (!Transform)
	{
		return false;
	}
	Transform->SetTransform(NewTransform);
	return true;
}

bool UMassUnitEntityManager::SetUnitDestination(FMassUnitHandle UnitHandle, const FVector& Destination, float AcceptanceRadius)
{
	if (!IsUnitValid(UnitHandle))
	{
		return false;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle NativeHandle = UnitHandle.EntityHandle.ToMassEntityHandle();
	FMassUnitTargetFragment* Target = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(NativeHandle);
	FMassUnitNavigationFragment* Navigation = EntityManager.GetFragmentDataPtr<FMassUnitNavigationFragment>(NativeHandle);
	if (!Target || !Navigation)
	{
		return false;
	}

	Target->TargetEntity.Invalidate();
	Target->TargetLocation = Destination;
	Target->bHasTargetLocation = true;
	Navigation->DestinationLocation = Destination;
	Navigation->PathPoints = {Destination};
	Navigation->CurrentPathIndex = 0;
	Navigation->AcceptanceRadius = FMath::Max(1.0f, AcceptanceRadius);
	Navigation->bPathRequested = false;
	Navigation->bPathValid = true;
	return true;
}

bool UMassUnitEntityManager::SetUnitTarget(FMassUnitHandle UnitHandle, FMassUnitHandle TargetUnit, float Priority)
{
	if (!IsUnitValid(UnitHandle) || !IsUnitValid(TargetUnit) || UnitHandle.EntityHandle == TargetUnit.EntityHandle)
	{
		return false;
	}
	FMassUnitTargetFragment* Target = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitTargetFragment>(UnitHandle.EntityHandle.ToMassEntityHandle());
	if (!Target)
	{
		return false;
	}
	Target->TargetEntity = TargetUnit.EntityHandle;
	Target->TargetLocation = FVector::ZeroVector;
	Target->bHasTargetLocation = false;
	Target->TargetPriority = Priority;
	return true;
}

void UMassUnitEntityManager::PruneInvalidUnits()
{
	if (!EntitySubsystem)
	{
		AllUnits.Reset();
		UnitTypeMap.Reset();
		TeamMap.Reset();
		return;
	}

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TArray<FMassUnitEntityHandle> InvalidHandles;
	for (const FMassUnitEntityHandle Handle : AllUnits)
	{
		if (!Handle.IsValid() || !EntityManager.IsEntityValid(Handle.ToMassEntityHandle()))
		{
			InvalidHandles.Add(Handle);
		}
	}
	for (const FMassUnitEntityHandle Handle : InvalidHandles)
	{
		RemoveHandleFromIndexes(Handle);
	}
}

void UMassUnitEntityManager::RemoveHandleFromIndexes(FMassUnitEntityHandle EntityHandle)
{
	AllUnits.Remove(EntityHandle);
	for (auto It = UnitTypeMap.CreateIterator(); It; ++It)
	{
		It.Value().Remove(EntityHandle);
		if (It.Value().IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
	for (auto It = TeamMap.CreateIterator(); It; ++It)
	{
		It.Value().Remove(EntityHandle);
		if (It.Value().IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}

bool UMassUnitEntityManager::ClearUnitTarget(FMassUnitHandle UnitHandle)
{
	if (!IsUnitValid(UnitHandle))
	{
		return false;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle NativeHandle = UnitHandle.EntityHandle.ToMassEntityHandle();
	if (FMassUnitTargetFragment* Target = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(NativeHandle))
	{
		Target->Clear();
	}
	if (FMassUnitNavigationFragment* Navigation = EntityManager.GetFragmentDataPtr<FMassUnitNavigationFragment>(NativeHandle))
	{
		Navigation->ResetPath();
	}
	return true;
}

bool UMassUnitEntityManager::GetUnitState(FMassUnitHandle UnitHandle, FMassUnitStateFragment& OutState) const
{
	if (!IsUnitValid(UnitHandle))
	{
		return false;
	}
	const FMassUnitStateFragment* State = EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FMassUnitStateFragment>(UnitHandle.EntityHandle.ToMassEntityHandle());
	if (!State)
	{
		return false;
	}
	OutState = *State;
	return true;
}

bool UMassUnitEntityManager::ApplyDamage(FMassUnitHandle UnitHandle, float Damage)
{
	if (!IsUnitValid(UnitHandle) || Damage <= 0.0f)
	{
		return false;
	}
	FMassUnitStateFragment* State = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitStateFragment>(UnitHandle.EntityHandle.ToMassEntityHandle());
	if (!State || State->CurrentState == EMassUnitState::Dead)
	{
		return false;
	}
	State->Health = FMath::Max(0.0f, State->Health - Damage);
	if (State->Health <= 0.0f)
	{
		State->CurrentState = EMassUnitState::Dead;
		State->StateTime = 0.0f;
	}
	return true;
}
