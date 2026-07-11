// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Navigation/MassUnitNavigationSystem.h"

#include "Config/MassUnitSystemSettings.h"
#include "Core/MassUnitSystemRuntime.h"
#include "Entity/MassUnitFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "NavigationSystem.h"

void UMassUnitNavigationSystem::Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem)
{
	World = InWorld;
	EntitySubsystem = InEntitySubsystem;
	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	MaxPathRequestsPerFrame = Settings ? FMath::Max(1, Settings->MaxPathRequestsPerFrame) : 100;
	UpdateNavigationData(InWorld);
}

void UMassUnitNavigationSystem::Deinitialize()
{
	TArray<uint32> PendingPathIds;
	PendingPathOwners.GenerateKeyArray(PendingPathIds);
	PendingPathOwners.Reset();
	if (NavigationSystem)
	{
		for (const uint32 PathId : PendingPathIds)
		{
			NavigationSystem->AbortAsyncFindPathRequest(PathId);
		}
	}
	PathRequestQueue.Reset();
	NavigationData = nullptr;
	NavigationSystem = nullptr;
	EntitySubsystem = nullptr;
	World = nullptr;
}

void UMassUnitNavigationSystem::UpdateNavigationData(UWorld* InWorld)
{
	World = InWorld;
	NavigationSystem = InWorld ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(InWorld) : nullptr;
	NavigationData = NavigationSystem ? NavigationSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
}

bool UMassUnitNavigationSystem::RequestPath(FMassUnitHandle UnitHandle, const FVector& Destination, float AcceptanceRadius)
{
	return RequestPathInternal(UnitHandle.EntityHandle, Destination, AcceptanceRadius);
}

bool UMassUnitNavigationSystem::RequestPathInternal(FMassUnitEntityHandle Entity, const FVector& Destination, float AcceptanceRadius)
{
	if (!IsEntityValid(Entity))
	{
		return false;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FMassUnitNavigationFragment* Navigation = EntityManager.GetFragmentDataPtr<FMassUnitNavigationFragment>(Entity.ToMassEntityHandle());
	if (!Navigation)
	{
		return false;
	}

	Navigation->DestinationLocation = Destination;
	Navigation->AcceptanceRadius = FMath::Max(1.0f, AcceptanceRadius);
	Navigation->PathPoints.Reset();
	Navigation->CurrentPathIndex = INDEX_NONE;
	Navigation->bPathRequested = true;
	Navigation->bPathValid = false;

	PathRequestQueue.RemoveAll([Entity](const FPathRequest& Existing) { return Existing.Entity == Entity; });
	TArray<uint32> SupersededPathIds;
	for (const TPair<uint32, FMassUnitEntityHandle>& Pair : PendingPathOwners)
	{
		if (Pair.Value == Entity)
		{
			SupersededPathIds.Add(Pair.Key);
		}
	}
	for (const uint32 PathId : SupersededPathIds)
	{
		PendingPathOwners.Remove(PathId);
		if (NavigationSystem)
		{
			NavigationSystem->AbortAsyncFindPathRequest(PathId);
		}
	}
	PathRequestQueue.Add({Entity, Destination, Navigation->AcceptanceRadius});
	return true;
}

void UMassUnitNavigationSystem::ProcessPathRequests()
{
	if (PathRequestQueue.IsEmpty())
	{
		return;
	}
	if (!NavigationSystem || !NavigationData)
	{
		UpdateNavigationData(World);
	}

	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	const bool bUseDirectFallback = !Settings || Settings->bFallbackToDirectPath;
	const int32 RequestCount = FMath::Min(MaxPathRequestsPerFrame, PathRequestQueue.Num());
	for (int32 Index = 0; Index < RequestCount; ++Index)
	{
		const FPathRequest Request = PathRequestQueue[0];
		PathRequestQueue.RemoveAt(0, 1, EAllowShrinking::No);
		if (!IsEntityValid(Request.Entity))
		{
			continue;
		}

		if (!NavigationSystem || !NavigationData)
		{
			if (bUseDirectFallback)
			{
				SetDirectPath(Request.Entity, Request.Destination, Request.AcceptanceRadius);
			}
			else
			{
				MarkPathFailed(Request.Entity);
			}
			continue;
		}

		const FMassUnitTransformFragment* Transform = EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FMassUnitTransformFragment>(Request.Entity.ToMassEntityHandle());
		if (!Transform)
		{
			MarkPathFailed(Request.Entity);
			continue;
		}
		FPathFindingQuery Query(nullptr, *NavigationData, Transform->GetTransform().GetLocation(), Request.Destination);
		const FNavPathQueryDelegate Delegate = FNavPathQueryDelegate::CreateUObject(this, &UMassUnitNavigationSystem::HandlePathRequestComplete);
		const uint32 PathId = NavigationSystem->FindPathAsync(FNavAgentProperties::DefaultProperties, MoveTemp(Query), Delegate);
		if (PathId != INVALID_NAVQUERYID)
		{
			PendingPathOwners.Add(PathId, Request.Entity);
		}
		else if (bUseDirectFallback)
		{
			SetDirectPath(Request.Entity, Request.Destination, Request.AcceptanceRadius);
		}
		else
		{
			MarkPathFailed(Request.Entity);
		}
	}
}

void UMassUnitNavigationSystem::HandlePathRequestComplete(uint32 PathId, ENavigationQueryResult::Type Result, FNavPathSharedPtr Path)
{
	FMassUnitEntityHandle Entity;
	if (!PendingPathOwners.RemoveAndCopyValue(PathId, Entity) || !IsEntityValid(Entity))
	{
		return;
	}
	if (Result == ENavigationQueryResult::Success && Path.IsValid())
	{
		UpdateEntityWithPath(Entity, Path);
		return;
	}

	FMassUnitNavigationFragment* Navigation = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitNavigationFragment>(Entity.ToMassEntityHandle());
	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	if (Navigation && (!Settings || Settings->bFallbackToDirectPath))
	{
		SetDirectPath(Entity, Navigation->DestinationLocation, Navigation->AcceptanceRadius);
	}
	else if (Navigation)
	{
		MarkPathFailed(Entity);
	}
}

void UMassUnitNavigationSystem::UpdateEntityWithPath(FMassUnitEntityHandle Entity, const FNavPathSharedPtr& Path)
{
	if (!IsEntityValid(Entity) || !Path.IsValid())
	{
		return;
	}
	FMassUnitNavigationFragment* Navigation = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitNavigationFragment>(Entity.ToMassEntityHandle());
	if (!Navigation)
	{
		return;
	}

	Navigation->PathPoints.Reset();
	for (const FNavPathPoint& Point : Path->GetPathPoints())
	{
		Navigation->PathPoints.Add(Point.Location);
	}
	if (Navigation->PathPoints.IsEmpty() || !Navigation->PathPoints.Last().Equals(Navigation->DestinationLocation, Navigation->AcceptanceRadius))
	{
		Navigation->PathPoints.Add(Navigation->DestinationLocation);
	}
	Navigation->CurrentPathIndex = Navigation->PathPoints.IsEmpty() ? INDEX_NONE : 0;
	Navigation->bPathRequested = false;
	Navigation->bPathValid = !Navigation->PathPoints.IsEmpty();
}

bool UMassUnitNavigationSystem::SetDirectPath(FMassUnitEntityHandle Entity, const FVector& Destination, float AcceptanceRadius)
{
	if (!IsEntityValid(Entity))
	{
		return false;
	}
	FMassUnitNavigationFragment* Navigation = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitNavigationFragment>(Entity.ToMassEntityHandle());
	if (!Navigation)
	{
		return false;
	}
	Navigation->DestinationLocation = Destination;
	Navigation->AcceptanceRadius = FMath::Max(1.0f, AcceptanceRadius);
	Navigation->PathPoints = {Destination};
	Navigation->CurrentPathIndex = 0;
	Navigation->bPathRequested = false;
	Navigation->bPathValid = true;
	return true;
}

void UMassUnitNavigationSystem::MarkPathFailed(FMassUnitEntityHandle Entity)
{
	if (!IsEntityValid(Entity))
	{
		return;
	}
	if (FMassUnitNavigationFragment* Navigation = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitNavigationFragment>(Entity.ToMassEntityHandle()))
	{
		Navigation->PathPoints.Reset();
		Navigation->CurrentPathIndex = INDEX_NONE;
		Navigation->bPathRequested = false;
		Navigation->bPathValid = false;
	}
}

bool UMassUnitNavigationSystem::IsEntityValid(FMassUnitEntityHandle Entity) const
{
	return EntitySubsystem && Entity.IsValid()
		&& EntitySubsystem->GetEntityManager().IsEntityValid(Entity.ToMassEntityHandle());
}
