// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Gameplay/MassUnitCrowdSystem.h"

#include "Config/MassUnitSystemSettings.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Entity/MassUnitFragments.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassUnitCommonFragments.h"
#include "Navigation/MassUnitNavigationSystem.h"
#include "Templates/UnrealTemplate.h"
#include "TimerManager.h"

void UMassUnitCrowdSystem::Initialize(
	UWorld* InWorld,
	UMassEntitySubsystem* InEntitySubsystem,
	UMassUnitEntityManager* InUnitManager,
	UMassUnitNavigationSystem* InNavigationSystem)
{
	World = InWorld;
	EntitySubsystem = InEntitySubsystem;
	UnitManager = InUnitManager;
	NavigationSystem = InNavigationSystem;

	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	UpdateInterval = Settings ? FMath::Max(0.02f, Settings->CrowdUpdateInterval) : 0.1f;
	MaxUnitsPerUpdate = Settings ? FMath::Max(1, Settings->MaxCrowdUnitsPerUpdate) : 500;
	SpatialCellSize = Settings ? FMath::Max(10.0f, Settings->CrowdSpatialCellSize) : 200.0f;
	SimulationLODDistances = Settings
		? Settings->CrowdSimulationLODDistances
		: TArray<float>{2500.0f, 5000.0f, 10000.0f};
	SimulationLODIntervalMultipliers = Settings
		? Settings->CrowdSimulationLODIntervalMultipliers
		: TArray<float>{1.0f, 2.0f, 4.0f, 8.0f};
	SimulationLODDistances.Sort();
	if (SimulationLODIntervalMultipliers.IsEmpty())
	{
		SimulationLODIntervalMultipliers.Add(1.0f);
	}
}

void UMassUnitCrowdSystem::Deinitialize()
{
	if (World)
	{
		World->GetTimerManager().ClearTimer(UpdateTimer);
	}

	TArray<int32> GroupHandles;
	Groups.GetKeys(GroupHandles);
	for (const int32 GroupHandle : GroupHandles)
	{
		UnregisterCrowdGroup(GroupHandle, false);
	}

	Groups.Reset();
	UnitToGroup.Reset();
	LastStats = {};
	NavigationSystem = nullptr;
	UnitManager = nullptr;
	EntitySubsystem = nullptr;
	World = nullptr;
}

int32 UMassUnitCrowdSystem::RegisterCrowdGroup(
	const TArray<FMassUnitHandle>& UnitHandles,
	FVector Center,
	const FMassUnitCrowdConfig& Config,
	bool bUseNavigation,
	float AcceptanceRadius)
{
	if (!World || !EntitySubsystem || !UnitManager || UnitHandles.IsEmpty())
	{
		return INDEX_NONE;
	}

	FCrowdGroup Group;
	Group.Handle = NextGroupHandle++;
	Group.Center = Center;
	Group.Config = SanitizeConfig(Config);
	Group.bUseNavigation = bUseNavigation;
	Group.AcceptanceRadius = FMath::Max(1.0f, AcceptanceRadius);

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const float CurrentTime = World->GetTimeSeconds();
	for (const FMassUnitHandle& UnitHandle : UnitHandles)
	{
		const FMassUnitEntityHandle Entity = UnitHandle.EntityHandle;
		if (!IsEntityValid(Entity))
		{
			continue;
		}

		RemoveUnitFromPreviousGroup(Entity);
		FMassUnitCrowdFragment* Crowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(Entity.ToMassEntityHandle());
		FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(Entity.ToMassEntityHandle());
		if (!Crowd || !State)
		{
			continue;
		}

		Crowd->Reset();
		Crowd->bEnabled = true;
		Crowd->GroupHandle = Group.Handle;
		Crowd->BaseMoveSpeed = State->MoveSpeed;
		Crowd->NextDecisionTime = CurrentTime;
		Group.Units.AddUnique(Entity);
		UnitToGroup.Add(Entity, Group.Handle);
	}

	if (Group.Units.IsEmpty())
	{
		return INDEX_NONE;
	}

	Groups.Add(Group.Handle, MoveTemp(Group));
	StartTimerIfNeeded();
	ForceCrowdGroupUpdate(Group.Handle);
	return Group.Handle;
}

bool UMassUnitCrowdSystem::UnregisterCrowdGroup(int32 CrowdGroupHandle, bool bStopUnits)
{
	FCrowdGroup Group;
	if (!Groups.RemoveAndCopyValue(CrowdGroupHandle, Group))
	{
		return false;
	}

	for (const FMassUnitEntityHandle Entity : Group.Units)
	{
		UnitToGroup.Remove(Entity);
		ResetCrowdFragment(Entity, bStopUnits);
	}
	StopTimerIfIdle();
	return true;
}

bool UMassUnitCrowdSystem::SetCrowdGroupPaused(int32 CrowdGroupHandle, bool bPaused, bool bStopUnits)
{
	FCrowdGroup* Group = Groups.Find(CrowdGroupHandle);
	if (!Group)
	{
		return false;
	}
	Group->bPaused = bPaused;

	if (bPaused && bStopUnits)
	{
		for (const FMassUnitEntityHandle Entity : Group->Units)
		{
			ClearMovement(Entity, true);
		}
	}
	else if (!bPaused && EntitySubsystem && World)
	{
		FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
		for (const FMassUnitEntityHandle Entity : Group->Units)
		{
			if (FMassUnitCrowdFragment* Crowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(Entity.ToMassEntityHandle()))
			{
				Crowd->NextDecisionTime = World->GetTimeSeconds();
			}
		}
		ForceCrowdGroupUpdate(CrowdGroupHandle);
	}
	return true;
}

bool UMassUnitCrowdSystem::SetCrowdGroupCenter(int32 CrowdGroupHandle, FVector NewCenter)
{
	if (FCrowdGroup* Group = Groups.Find(CrowdGroupHandle))
	{
		Group->Center = NewCenter;
		return true;
	}
	return false;
}

bool UMassUnitCrowdSystem::ForceCrowdGroupUpdate(int32 CrowdGroupHandle)
{
	FCrowdGroup* Group = Groups.Find(CrowdGroupHandle);
	if (!Group || Group->bPaused || !World || bUpdatingCrowds)
	{
		return false;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingCrowds, true);

	PruneInvalidUnits();
	Group = Groups.Find(CrowdGroupHandle);
	if (!Group)
	{
		return false;
	}

	TArray<FSpatialEntry> Entries;
	TMap<FIntPoint, TArray<int32>> Cells;
	TMap<FMassUnitEntityHandle, int32> EntryByEntity;
	TArray<FVector> ObserverLocations;
	BuildSpatialData(Entries, Cells, EntryByEntity);
	BuildObserverLocations(ObserverLocations);

	LastStats = {};
	RefreshPopulationStats(Entries);
	const float CurrentTime = World->GetTimeSeconds();
	for (const FSpatialEntry& Entry : Entries)
	{
		if (Entry.GroupHandle == CrowdGroupHandle)
		{
			FCrowdGroup* CurrentGroup = Groups.Find(CrowdGroupHandle);
			if (!CurrentGroup || CurrentGroup->bPaused)
			{
				break;
			}
			UpdateUnit(Entry, *CurrentGroup, Entries, Cells, EntryByEntity, ObserverLocations, CurrentTime, true);
			++LastStats.UnitsUpdated;
		}
	}
	PruneInvalidUnits();
	RefreshPopulationStats(Entries);
	return true;
}

bool UMassUnitCrowdSystem::IsCrowdGroupRegistered(int32 CrowdGroupHandle) const
{
	return Groups.Contains(CrowdGroupHandle);
}

int32 UMassUnitCrowdSystem::GetCrowdGroupUnitCount(int32 CrowdGroupHandle) const
{
	const FCrowdGroup* Group = Groups.Find(CrowdGroupHandle);
	return Group ? Group->Units.Num() : 0;
}

void UMassUnitCrowdSystem::StartTimerIfNeeded()
{
	if (World && !Groups.IsEmpty() && !World->GetTimerManager().IsTimerActive(UpdateTimer))
	{
		World->GetTimerManager().SetTimer(UpdateTimer, this, &UMassUnitCrowdSystem::UpdateCrowds, UpdateInterval, true);
	}
}

void UMassUnitCrowdSystem::StopTimerIfIdle()
{
	if (World && Groups.IsEmpty())
	{
		World->GetTimerManager().ClearTimer(UpdateTimer);
		UpdateCursor = 0;
	}
}

void UMassUnitCrowdSystem::UpdateCrowds()
{
	if (!World || !EntitySubsystem || !UnitManager || bUpdatingCrowds)
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingCrowds, true);

	PruneInvalidUnits();
	if (Groups.IsEmpty())
	{
		StopTimerIfIdle();
		return;
	}

	TArray<FSpatialEntry> Entries;
	TMap<FIntPoint, TArray<int32>> Cells;
	TMap<FMassUnitEntityHandle, int32> EntryByEntity;
	TArray<FVector> ObserverLocations;
	BuildSpatialData(Entries, Cells, EntryByEntity);
	BuildObserverLocations(ObserverLocations);

	LastStats = {};
	RefreshPopulationStats(Entries);

	if (Entries.IsEmpty())
	{
		return;
	}

	const int32 UnitsToUpdate = FMath::Min(MaxUnitsPerUpdate, Entries.Num());
	const int32 StartIndex = UpdateCursor % Entries.Num();
	const float CurrentTime = World->GetTimeSeconds();
	for (int32 Offset = 0; Offset < UnitsToUpdate; ++Offset)
	{
		const FSpatialEntry& Entry = Entries[(StartIndex + Offset) % Entries.Num()];
		FCrowdGroup* Group = Groups.Find(Entry.GroupHandle);
		if (!Group || Group->bPaused)
		{
			continue;
		}
		UpdateUnit(Entry, *Group, Entries, Cells, EntryByEntity, ObserverLocations, CurrentTime, false);
		++LastStats.UnitsUpdated;
	}
	UpdateCursor = (StartIndex + UnitsToUpdate) % Entries.Num();
	PruneInvalidUnits();
	RefreshPopulationStats(Entries);
}

void UMassUnitCrowdSystem::PruneInvalidUnits()
{
	TArray<int32> EmptyGroups;
	for (TPair<int32, FCrowdGroup>& Pair : Groups)
	{
		Pair.Value.Units.RemoveAll([this](const FMassUnitEntityHandle Entity)
		{
			if (IsEntityValid(Entity))
			{
				return false;
			}
			UnitToGroup.Remove(Entity);
			return true;
		});
		if (Pair.Value.Units.IsEmpty())
		{
			EmptyGroups.Add(Pair.Key);
		}
	}
	for (const int32 GroupHandle : EmptyGroups)
	{
		Groups.Remove(GroupHandle);
	}
}

void UMassUnitCrowdSystem::BuildSpatialData(
	TArray<FSpatialEntry>& OutEntries,
	TMap<FIntPoint, TArray<int32>>& OutCells,
	TMap<FMassUnitEntityHandle, int32>& OutEntryByEntity) const
{
	if (!EntitySubsystem)
	{
		return;
	}

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	for (const TPair<int32, FCrowdGroup>& Pair : Groups)
	{
		for (const FMassUnitEntityHandle Entity : Pair.Value.Units)
		{
			if (!IsEntityValid(Entity))
			{
				continue;
			}
			const FMassUnitTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FMassUnitTransformFragment>(Entity.ToMassEntityHandle());
			if (!Transform)
			{
				continue;
			}

			FSpatialEntry& Entry = OutEntries.AddDefaulted_GetRef();
			Entry.Entity = Entity;
			Entry.Location = Transform->GetTransform().GetLocation();
			Entry.GroupHandle = Pair.Key;
			const int32 EntryIndex = OutEntries.Num() - 1;
			OutEntryByEntity.Add(Entity, EntryIndex);
			OutCells.FindOrAdd(ToSpatialCell(Entry.Location)).Add(EntryIndex);
		}
	}
}

void UMassUnitCrowdSystem::BuildObserverLocations(TArray<FVector>& OutObserverLocations) const
{
	if (!World)
	{
		return;
	}
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* Controller = It->Get();
		if (!Controller)
		{
			continue;
		}
		if (const APawn* Pawn = Controller->GetPawn())
		{
			OutObserverLocations.Add(Pawn->GetActorLocation());
		}
		else
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
			OutObserverLocations.Add(ViewLocation);
		}
	}
}

void UMassUnitCrowdSystem::RefreshPopulationStats(const TArray<FSpatialEntry>& Entries)
{
	LastStats.RegisteredGroups = Groups.Num();
	LastStats.RegisteredUnits = 0;
	LastStats.ActiveUnits = 0;
	LastStats.SleepingUnits = 0;
	if (!EntitySubsystem)
	{
		return;
	}

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	for (const FSpatialEntry& Entry : Entries)
	{
		if (!IsEntityValid(Entry.Entity))
		{
			continue;
		}
		++LastStats.RegisteredUnits;
		const FMassUnitCrowdFragment* Crowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(Entry.Entity.ToMassEntityHandle());
		if (Crowd && Crowd->bSleeping)
		{
			++LastStats.SleepingUnits;
		}
		else
		{
			++LastStats.ActiveUnits;
		}
	}
}

void UMassUnitCrowdSystem::UpdateUnit(
	const FSpatialEntry& Entry,
	FCrowdGroup& Group,
	const TArray<FSpatialEntry>& Entries,
	const TMap<FIntPoint, TArray<int32>>& Cells,
	const TMap<FMassUnitEntityHandle, int32>& EntryByEntity,
	const TArray<FVector>& ObserverLocations,
	float CurrentTime,
	bool bForceDecision)
{
	if (!IsEntityValid(Entry.Entity) || !EntitySubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle NativeHandle = Entry.Entity.ToMassEntityHandle();
	FMassUnitCrowdFragment* Crowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(NativeHandle);
	FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(NativeHandle);
	FMassUnitTargetFragment* Target = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(NativeHandle);
	FMassUnitNavigationFragment* Navigation = EntityManager.GetFragmentDataPtr<FMassUnitNavigationFragment>(NativeHandle);
	if (!Crowd || !State || !Target || !Navigation || !Crowd->bEnabled)
	{
		return;
	}

	float ObserverDistanceSquared = 0.0f;
	const int32 LODLevel = CalculateSimulationLOD(Entry.Location, ObserverLocations, ObserverDistanceSquared);
	const float LODMultiplier = GetSimulationIntervalMultiplier(LODLevel);
	Crowd->SimulationLOD = LODLevel;

	const bool bShouldSleep = !ObserverLocations.IsEmpty()
		&& Group.Config.MaxSimulationDistance > 0.0f
		&& ObserverDistanceSquared > FMath::Square(Group.Config.MaxSimulationDistance);
	if (bShouldSleep)
	{
		SetUnitSleeping(Entry.Entity, true, CurrentTime);
		return;
	}
	if (Crowd->bSleeping)
	{
		SetUnitSleeping(Entry.Entity, false, CurrentTime);
		bForceDecision = true;
	}

	if (State->CurrentState == EMassUnitState::Dead || State->CurrentState == EMassUnitState::Stunned)
	{
		Crowd->SteeringDirection = FVector::ZeroVector;
		return;
	}

	if (Crowd->InteractionEndTime > CurrentTime)
	{
		Crowd->SteeringDirection = FVector::ZeroVector;
		return;
	}
	if (Crowd->InteractionEndTime > 0.0f)
	{
		Crowd->InteractionEndTime = 0.0f;
		Crowd->InteractionPartner.Invalidate();
		if (State->CurrentState == EMassUnitState::Interacting)
		{
			State->CurrentState = EMassUnitState::Idle;
			State->StateTime = 0.0f;
		}
	}

	if (bForceDecision || CurrentTime >= Crowd->NextSteeringUpdateTime)
	{
		Crowd->SteeringDirection = CalculateSeparation(Entry, Group, Entries, Cells);
		Crowd->NextSteeringUpdateTime = CurrentTime + (UpdateInterval * LODMultiplier);
	}

	const bool bHasMovementTarget = Target->bHasTargetLocation || Navigation->bPathRequested || Navigation->bPathValid;
	if (bHasMovementTarget)
	{
		const FVector Destination = Target->bHasTargetLocation ? Target->TargetLocation : Navigation->DestinationLocation;
		const bool bReached = FVector::DistSquared2D(Entry.Location, Destination) <= FMath::Square(Group.AcceptanceRadius);
		const bool bTimedOut = CurrentTime >= Crowd->NextDecisionTime;
		if (!bReached && !bTimedOut && !bForceDecision)
		{
			return;
		}

		ClearMovement(Entry.Entity, true);
		Crowd->NextDecisionTime = CurrentTime;
		if (!bForceDecision && bReached)
		{
			FRandomStream IdleStream = MakeRandomStream(Entry.Entity, Group, Crowd->DecisionSequence++);
			Crowd->NextDecisionTime = CurrentTime
				+ IdleStream.FRandRange(Group.Config.MinIdleTime, Group.Config.MaxIdleTime) * LODMultiplier;
			return;
		}
	}

	if (!bForceDecision && CurrentTime < Crowd->NextDecisionTime)
	{
		return;
	}

	FRandomStream RandomStream = MakeRandomStream(Entry.Entity, Group, Crowd->DecisionSequence++);
	if (Group.Config.bEnableInteractions
		&& Group.Config.InteractionChance > 0.0f
		&& RandomStream.FRand() <= Group.Config.InteractionChance)
	{
		const FMassUnitEntityHandle Partner = FindInteractionPartner(Entry, Group, Entries, Cells, CurrentTime);
		if (Partner.IsValid() && EntryByEntity.Contains(Partner))
		{
			BeginInteraction(Entry.Entity, Partner, Group, CurrentTime, RandomStream);
			return;
		}
	}

	AssignRandomDestination(Entry.Entity, Group, CurrentTime, LODMultiplier, RandomStream);
}

FVector UMassUnitCrowdSystem::CalculateSeparation(
	const FSpatialEntry& Entry,
	const FCrowdGroup& Group,
	const TArray<FSpatialEntry>& Entries,
	const TMap<FIntPoint, TArray<int32>>& Cells)
{
	if (!Group.Config.bEnableSeparation || Group.Config.SeparationWeight <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	const float Radius = Group.Config.SeparationRadius;
	const float RadiusSquared = FMath::Square(Radius);
	const int32 CellRange = FMath::Max(1, FMath::CeilToInt(Radius / SpatialCellSize));
	const FIntPoint CenterCell = ToSpatialCell(Entry.Location);
	FVector Separation = FVector::ZeroVector;
	for (int32 CellX = CenterCell.X - CellRange; CellX <= CenterCell.X + CellRange; ++CellX)
	{
		for (int32 CellY = CenterCell.Y - CellRange; CellY <= CenterCell.Y + CellRange; ++CellY)
		{
			const TArray<int32>* CellEntries = Cells.Find(FIntPoint(CellX, CellY));
			if (!CellEntries)
			{
				continue;
			}
			for (const int32 EntryIndex : *CellEntries)
			{
				if (!Entries.IsValidIndex(EntryIndex) || Entries[EntryIndex].Entity == Entry.Entity)
				{
					continue;
				}
				++LastStats.NeighborChecks;
				const FVector Away = Entry.Location - Entries[EntryIndex].Location;
				const float DistanceSquared = Away.SizeSquared2D();
				if (DistanceSquared <= UE_SMALL_NUMBER || DistanceSquared > RadiusSquared)
				{
					continue;
				}
				const float Distance = FMath::Sqrt(DistanceSquared);
				Separation += Away.GetSafeNormal2D() * (1.0f - (Distance / Radius));
			}
		}
	}
	return Separation.GetClampedToMaxSize(1.0f) * Group.Config.SeparationWeight;
}

FMassUnitEntityHandle UMassUnitCrowdSystem::FindInteractionPartner(
	const FSpatialEntry& Entry,
	const FCrowdGroup& Group,
	const TArray<FSpatialEntry>& Entries,
	const TMap<FIntPoint, TArray<int32>>& Cells,
	float CurrentTime)
{
	if (!EntitySubsystem)
	{
		return {};
	}

	const float RadiusSquared = FMath::Square(Group.Config.InteractionRadius);
	const int32 CellRange = FMath::Max(1, FMath::CeilToInt(Group.Config.InteractionRadius / SpatialCellSize));
	const FIntPoint CenterCell = ToSpatialCell(Entry.Location);
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	FMassUnitEntityHandle BestPartner;
	float BestDistanceSquared = RadiusSquared;
	for (int32 CellX = CenterCell.X - CellRange; CellX <= CenterCell.X + CellRange; ++CellX)
	{
		for (int32 CellY = CenterCell.Y - CellRange; CellY <= CenterCell.Y + CellRange; ++CellY)
		{
			const TArray<int32>* CellEntries = Cells.Find(FIntPoint(CellX, CellY));
			if (!CellEntries)
			{
				continue;
			}
			for (const int32 EntryIndex : *CellEntries)
			{
				if (!Entries.IsValidIndex(EntryIndex))
				{
					continue;
				}
				const FSpatialEntry& Candidate = Entries[EntryIndex];
				if (Candidate.Entity == Entry.Entity || Candidate.GroupHandle != Group.Handle)
				{
					continue;
				}
				++LastStats.NeighborChecks;
				const float DistanceSquared = FVector::DistSquared2D(Entry.Location, Candidate.Location);
				if (DistanceSquared > BestDistanceSquared)
				{
					continue;
				}
				const FMassUnitCrowdFragment* CandidateCrowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(Candidate.Entity.ToMassEntityHandle());
				const FMassUnitStateFragment* CandidateState = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(Candidate.Entity.ToMassEntityHandle());
				if (!CandidateCrowd || !CandidateState || !CandidateCrowd->bEnabled || CandidateCrowd->bSleeping
					|| CandidateCrowd->InteractionEndTime > CurrentTime
					|| CandidateState->CurrentState == EMassUnitState::Dead
					|| CandidateState->CurrentState == EMassUnitState::Stunned)
				{
					continue;
				}
				BestPartner = Candidate.Entity;
				BestDistanceSquared = DistanceSquared;
			}
		}
	}
	return BestPartner;
}

void UMassUnitCrowdSystem::BeginInteraction(
	FMassUnitEntityHandle UnitA,
	FMassUnitEntityHandle UnitB,
	FCrowdGroup& Group,
	float CurrentTime,
	FRandomStream& RandomStream)
{
	if (!EntitySubsystem || !IsEntityValid(UnitA) || !IsEntityValid(UnitB))
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const float Duration = RandomStream.FRandRange(Group.Config.MinInteractionTime, Group.Config.MaxInteractionTime);
	const float EndTime = CurrentTime + Duration;
	const FMassUnitTransformFragment* TransformA = EntityManager.GetFragmentDataPtr<FMassUnitTransformFragment>(UnitA.ToMassEntityHandle());
	const FMassUnitTransformFragment* TransformB = EntityManager.GetFragmentDataPtr<FMassUnitTransformFragment>(UnitB.ToMassEntityHandle());
	if (!TransformA || !TransformB)
	{
		return;
	}

	const FVector DirectionA = (TransformB->GetTransform().GetLocation() - TransformA->GetTransform().GetLocation()).GetSafeNormal2D();
	const FVector DirectionB = -DirectionA;
	const TPair<FMassUnitEntityHandle, FVector> Units[] = {
		{UnitA, DirectionA},
		{UnitB, DirectionB}
	};
	for (const TPair<FMassUnitEntityHandle, FVector>& Pair : Units)
	{
		ClearMovement(Pair.Key, false);
		FMassUnitCrowdFragment* Crowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(Pair.Key.ToMassEntityHandle());
		FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(Pair.Key.ToMassEntityHandle());
		FMassUnitLookAtFragment* LookAt = EntityManager.GetFragmentDataPtr<FMassUnitLookAtFragment>(Pair.Key.ToMassEntityHandle());
		if (Crowd && State && LookAt)
		{
			Crowd->InteractionPartner = Pair.Key == UnitA ? UnitB : UnitA;
			Crowd->InteractionEndTime = EndTime;
			Crowd->NextDecisionTime = EndTime + RandomStream.FRandRange(Group.Config.MinIdleTime, Group.Config.MaxIdleTime);
			Crowd->SteeringDirection = FVector::ZeroVector;
			State->CurrentState = EMassUnitState::Interacting;
			State->StateTime = 0.0f;
			LookAt->Direction = Pair.Value;
		}
	}

	++LastStats.InteractionsStarted;
	OnCrowdInteractionStarted.Broadcast(FMassUnitHandle(UnitA), FMassUnitHandle(UnitB));
#if ENABLE_DRAW_DEBUG
	if (Group.Config.bEnableVisualDebug && World)
	{
		DrawDebugLine(
			World,
			TransformA->GetTransform().GetLocation(),
			TransformB->GetTransform().GetLocation(),
			FColor::Orange,
			false,
			FMath::Max(0.1f, Duration),
			0,
			2.0f);
	}
#endif
}

bool UMassUnitCrowdSystem::AssignRandomDestination(
	FMassUnitEntityHandle Entity,
	FCrowdGroup& Group,
	float CurrentTime,
	float LODIntervalMultiplier,
	FRandomStream& RandomStream)
{
	if (!EntitySubsystem || !UnitManager || !IsEntityValid(Entity))
	{
		return false;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FMassUnitCrowdFragment* Crowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(Entity.ToMassEntityHandle());
	FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(Entity.ToMassEntityHandle());
	const FMassUnitTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FMassUnitTransformFragment>(Entity.ToMassEntityHandle());
	if (!Crowd || !State || !Transform)
	{
		return false;
	}

	const FVector CurrentLocation = Transform->GetTransform().GetLocation();
	FVector Destination = CurrentLocation;
	for (int32 Attempt = 0; Attempt < 4; ++Attempt)
	{
		const float Angle = RandomStream.FRandRange(0.0f, 2.0f * UE_PI);
		const float Radius = FMath::Sqrt(RandomStream.FRand()) * Group.Config.WanderRadius;
		Destination = Group.Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
		Destination.Z = CurrentLocation.Z;
		if (FVector::DistSquared2D(CurrentLocation, Destination) >= FMath::Square(Group.Config.MinWanderDistance))
		{
			break;
		}
	}

	const bool bAssigned = Group.bUseNavigation && NavigationSystem
		? NavigationSystem->RequestPathInternal(Entity, Destination, Group.AcceptanceRadius)
		: UnitManager->SetUnitDestination(FMassUnitHandle(Entity), Destination, Group.AcceptanceRadius);
	if (!bAssigned)
	{
		Crowd->NextDecisionTime = CurrentTime + UpdateInterval * LODIntervalMultiplier;
		return false;
	}

	const float SpeedMultiplier = RandomStream.FRandRange(
		Group.Config.MinMoveSpeedMultiplier,
		Group.Config.MaxMoveSpeedMultiplier);
	State->MoveSpeed = Crowd->BaseMoveSpeed * SpeedMultiplier;
	Crowd->LastDestination = Destination;
	Crowd->NextDecisionTime = CurrentTime + Group.Config.MaxMoveTime * LODIntervalMultiplier;
	++LastStats.DestinationsAssigned;

#if ENABLE_DRAW_DEBUG
	if (Group.Config.bEnableVisualDebug && World)
	{
		DrawDebugDirectionalArrow(World, CurrentLocation, Destination, 60.0f, FColor::Cyan, false, 1.0f, 0, 1.5f);
		DrawDebugCircle(World, Group.Center, Group.Config.WanderRadius, 64, FColor::White, false, 1.0f, 0, 1.0f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	}
#endif
	return true;
}

void UMassUnitCrowdSystem::ClearMovement(FMassUnitEntityHandle Entity, bool bSetIdle) const
{
	if (!EntitySubsystem || !IsEntityValid(Entity))
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle NativeHandle = Entity.ToMassEntityHandle();
	if (NavigationSystem)
	{
		NavigationSystem->CancelPathInternal(Entity);
	}
	if (FMassUnitTargetFragment* Target = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(NativeHandle))
	{
		Target->Clear();
	}
	if (FMassUnitNavigationFragment* Navigation = EntityManager.GetFragmentDataPtr<FMassUnitNavigationFragment>(NativeHandle))
	{
		Navigation->ResetPath();
	}
	if (FMassUnitVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassUnitVelocityFragment>(NativeHandle))
	{
		Velocity->Value = FVector::ZeroVector;
	}
	if (bSetIdle)
	{
		if (FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(NativeHandle))
		{
			if (State->CurrentState != EMassUnitState::Dead && State->CurrentState != EMassUnitState::Stunned)
			{
				State->CurrentState = EMassUnitState::Idle;
				State->StateTime = 0.0f;
			}
		}
	}
}

void UMassUnitCrowdSystem::SetUnitSleeping(FMassUnitEntityHandle Entity, bool bSleeping, float CurrentTime) const
{
	if (!EntitySubsystem || !IsEntityValid(Entity))
	{
		return;
	}
	FMassUnitCrowdFragment* Crowd = EntitySubsystem->GetMutableEntityManager().GetFragmentDataPtr<FMassUnitCrowdFragment>(Entity.ToMassEntityHandle());
	if (!Crowd || Crowd->bSleeping == bSleeping)
	{
		return;
	}
	Crowd->bSleeping = bSleeping;
	Crowd->SteeringDirection = FVector::ZeroVector;
	Crowd->NextDecisionTime = CurrentTime;
	if (bSleeping)
	{
		ClearMovement(Entity, true);
	}
}

void UMassUnitCrowdSystem::ResetCrowdFragment(FMassUnitEntityHandle Entity, bool bStopUnit) const
{
	if (!EntitySubsystem || !IsEntityValid(Entity))
	{
		return;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle NativeHandle = Entity.ToMassEntityHandle();
	FMassUnitCrowdFragment* Crowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(NativeHandle);
	FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(NativeHandle);
	if (Crowd && State && Crowd->BaseMoveSpeed > 0.0f)
	{
		State->MoveSpeed = Crowd->BaseMoveSpeed;
	}
	if (Crowd)
	{
		Crowd->Reset();
	}
	if (bStopUnit)
	{
		ClearMovement(Entity, true);
	}
}

void UMassUnitCrowdSystem::RemoveUnitFromPreviousGroup(FMassUnitEntityHandle Entity)
{
	int32 PreviousGroupHandle = INDEX_NONE;
	if (!UnitToGroup.RemoveAndCopyValue(Entity, PreviousGroupHandle))
	{
		return;
	}
	if (FCrowdGroup* PreviousGroup = Groups.Find(PreviousGroupHandle))
	{
		PreviousGroup->Units.Remove(Entity);
		if (PreviousGroup->Units.IsEmpty())
		{
			Groups.Remove(PreviousGroupHandle);
		}
	}
}

FRandomStream UMassUnitCrowdSystem::MakeRandomStream(
	FMassUnitEntityHandle Entity,
	const FCrowdGroup& Group,
	int32 DecisionSequence) const
{
	uint32 Seed = HashCombine(::GetTypeHash(Group.Config.RandomSeed), GetTypeHash(Entity));
	Seed = HashCombine(Seed, ::GetTypeHash(DecisionSequence));
	return FRandomStream(static_cast<int32>(Seed));
}

int32 UMassUnitCrowdSystem::CalculateSimulationLOD(
	const FVector& Location,
	const TArray<FVector>& ObserverLocations,
	float& OutDistanceSquared) const
{
	if (ObserverLocations.IsEmpty())
	{
		OutDistanceSquared = 0.0f;
		return 0;
	}

	OutDistanceSquared = TNumericLimits<float>::Max();
	for (const FVector& ObserverLocation : ObserverLocations)
	{
		OutDistanceSquared = FMath::Min(OutDistanceSquared, FVector::DistSquared(Location, ObserverLocation));
	}
	for (int32 Index = 0; Index < SimulationLODDistances.Num(); ++Index)
	{
		if (OutDistanceSquared <= FMath::Square(FMath::Max(0.0f, SimulationLODDistances[Index])))
		{
			return Index;
		}
	}
	return SimulationLODDistances.Num();
}

float UMassUnitCrowdSystem::GetSimulationIntervalMultiplier(int32 LODLevel) const
{
	const int32 Index = FMath::Clamp(LODLevel, 0, SimulationLODIntervalMultipliers.Num() - 1);
	return FMath::Max(1.0f, SimulationLODIntervalMultipliers[Index]);
}

FIntPoint UMassUnitCrowdSystem::ToSpatialCell(const FVector& Location) const
{
	return FIntPoint(
		FMath::FloorToInt(Location.X / SpatialCellSize),
		FMath::FloorToInt(Location.Y / SpatialCellSize));
}

FMassUnitCrowdConfig UMassUnitCrowdSystem::SanitizeConfig(const FMassUnitCrowdConfig& Config)
{
	FMassUnitCrowdConfig Result = Config;
	Result.WanderRadius = FMath::Max(1.0f, Result.WanderRadius);
	Result.MinWanderDistance = FMath::Clamp(Result.MinWanderDistance, 0.0f, Result.WanderRadius);
	Result.MinIdleTime = FMath::Max(0.0f, Result.MinIdleTime);
	Result.MaxIdleTime = FMath::Max(Result.MinIdleTime, Result.MaxIdleTime);
	Result.MaxMoveTime = FMath::Max(0.1f, Result.MaxMoveTime);
	Result.MinMoveSpeedMultiplier = FMath::Clamp(Result.MinMoveSpeedMultiplier, 0.05f, 4.0f);
	Result.MaxMoveSpeedMultiplier = FMath::Clamp(Result.MaxMoveSpeedMultiplier, Result.MinMoveSpeedMultiplier, 4.0f);
	Result.SeparationRadius = FMath::Max(1.0f, Result.SeparationRadius);
	Result.SeparationWeight = FMath::Clamp(Result.SeparationWeight, 0.0f, 4.0f);
	Result.InteractionChance = FMath::Clamp(Result.InteractionChance, 0.0f, 1.0f);
	Result.InteractionRadius = FMath::Max(1.0f, Result.InteractionRadius);
	Result.MinInteractionTime = FMath::Max(0.0f, Result.MinInteractionTime);
	Result.MaxInteractionTime = FMath::Max(Result.MinInteractionTime, Result.MaxInteractionTime);
	Result.MaxSimulationDistance = FMath::Max(0.0f, Result.MaxSimulationDistance);
	return Result;
}

bool UMassUnitCrowdSystem::IsEntityValid(FMassUnitEntityHandle Entity) const
{
	return EntitySubsystem && Entity.IsValid()
		&& EntitySubsystem->GetEntityManager().IsEntityValid(Entity.ToMassEntityHandle());
}
