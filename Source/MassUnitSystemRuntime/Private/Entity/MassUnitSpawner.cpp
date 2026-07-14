// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Core/MassUnitSubsystem.h"
#include "Core/MassUnitSystemRuntime.h"
#include "DrawDebugHelpers.h"
#include "Entity/UnitTemplate.h"
#include "Navigation/MassUnitNavigationSystem.h"

AMassUnitSpawner::AMassUnitSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(SceneRoot);
	DirectionArrow->ArrowColor = FColor::Cyan;
	DirectionArrow->ArrowSize = 2.0f;
	DirectionArrow->bIsScreenSizeScaled = true;
}

void AMassUnitSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!bSpawnOnBeginPlay || (bSpawnOnAuthorityOnly && GetNetMode() == NM_Client))
	{
		return;
	}

	SpawnUnits();
	if (bEnableCrowdSimulation)
	{
		StartCrowdSimulation();
	}
	else if (bMoveOnBeginPlay)
	{
		MoveSpawnedUnitsByOffset(DestinationOffset);
	}
}

void AMassUnitSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bDestroySpawnedUnitsOnEndPlay)
	{
		DestroySpawnedUnits();
	}
	Super::EndPlay(EndPlayReason);
}

TArray<FMassUnitHandle> AMassUnitSpawner::SpawnUnits()
{
	TArray<FMassUnitHandle> NewUnits;
	if (bSpawnOnAuthorityOnly && GetNetMode() == NM_Client)
	{
		UE_LOG(LogMassUnitSystem, Verbose, TEXT("%s skipped Mass unit spawning on a non-authority world"), *GetName());
		return NewUnits;
	}

	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitEntityManager* UnitManager = UnitSubsystem ? UnitSubsystem->GetUnitManager() : nullptr;
	if (!UnitManager)
	{
		UE_LOG(LogMassUnitSystem, Warning, TEXT("%s could not resolve the Mass Unit world subsystem"), *GetName());
		return NewUnits;
	}

	const int32 RequestedCount = FMath::Max(0, UnitCount);
	NewUnits.Reserve(RequestedCount);
	for (int32 UnitIndex = 0; UnitIndex < RequestedCount; ++UnitIndex)
	{
		const FVector SpawnLocation = GetActorTransform().TransformPosition(CalculateLocalGridOffset(UnitIndex, RequestedCount));
		const FTransform SpawnTransform(GetActorQuat(), SpawnLocation, GetActorScale3D());
		const FMassUnitHandle UnitHandle = UnitTemplate
			? UnitManager->CreateUnitFromTemplate(UnitTemplate, SpawnTransform)
			: UnitManager->CreateDefaultUnit(SpawnTransform);
		if (!UnitManager->IsUnitValid(UnitHandle))
		{
			break;
		}
		SpawnedUnits.Add(UnitHandle);
		NewUnits.Add(UnitHandle);
		DrawSpawnDebug(SpawnLocation);
	}

	UE_LOG(LogMassUnitSystem, Log, TEXT("%s spawned %d Mass units"), *GetName(), NewUnits.Num());
	return NewUnits;
}

int32 AMassUnitSpawner::MoveSpawnedUnitsByOffset(FVector LocalOffset)
{
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitEntityManager* UnitManager = UnitSubsystem ? UnitSubsystem->GetUnitManager() : nullptr;
	if (!UnitManager)
	{
		return 0;
	}

	const FVector WorldOffset = GetActorTransform().TransformVectorNoScale(LocalOffset);
	int32 CommandedCount = 0;
	for (const FMassUnitHandle UnitHandle : SpawnedUnits)
	{
		FTransform CurrentTransform;
		if (!UnitManager->GetUnitTransform(UnitHandle, CurrentTransform))
		{
			continue;
		}
		const FVector Destination = CurrentTransform.GetLocation() + WorldOffset;
		if (CommandUnit(UnitHandle, Destination))
		{
			++CommandedCount;
			DrawCommandDebug(CurrentTransform.GetLocation(), Destination);
		}
	}
	return CommandedCount;
}

int32 AMassUnitSpawner::CommandSpawnedUnitsToLocation(FVector DestinationCenter, bool bPreserveSpacing)
{
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitEntityManager* UnitManager = UnitSubsystem ? UnitSubsystem->GetUnitManager() : nullptr;
	if (!UnitManager)
	{
		return 0;
	}

	struct FUnitLocation
	{
		FMassUnitHandle Handle;
		FVector Location = FVector::ZeroVector;
	};
	TArray<FUnitLocation> UnitLocations;
	FVector CurrentCenter = FVector::ZeroVector;
	for (const FMassUnitHandle UnitHandle : SpawnedUnits)
	{
		FTransform CurrentTransform;
		if (UnitManager->GetUnitTransform(UnitHandle, CurrentTransform))
		{
			UnitLocations.Add({UnitHandle, CurrentTransform.GetLocation()});
			CurrentCenter += CurrentTransform.GetLocation();
		}
	}
	if (UnitLocations.IsEmpty())
	{
		return 0;
	}
	CurrentCenter /= static_cast<float>(UnitLocations.Num());

	int32 CommandedCount = 0;
	for (const FUnitLocation& Unit : UnitLocations)
	{
		const FVector Destination = bPreserveSpacing
			? DestinationCenter + (Unit.Location - CurrentCenter)
			: DestinationCenter;
		if (CommandUnit(Unit.Handle, Destination))
		{
			++CommandedCount;
			DrawCommandDebug(Unit.Location, Destination);
		}
	}
	return CommandedCount;
}

void AMassUnitSpawner::DestroySpawnedUnits()
{
	StopCrowdSimulation(true);
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitEntityManager* UnitManager = UnitSubsystem ? UnitSubsystem->GetUnitManager() : nullptr;
	if (UnitManager)
	{
		for (const FMassUnitHandle UnitHandle : SpawnedUnits)
		{
			if (UnitManager->IsUnitValid(UnitHandle))
			{
				UnitManager->DestroyUnit(UnitHandle);
			}
		}
	}
	SpawnedUnits.Reset();
}

TArray<FMassUnitHandle> AMassUnitSpawner::GetValidSpawnedUnits() const
{
	TArray<FMassUnitHandle> ValidUnits;
	const UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	const UMassUnitEntityManager* UnitManager = UnitSubsystem ? UnitSubsystem->GetUnitManager() : nullptr;
	if (!UnitManager)
	{
		return ValidUnits;
	}
	for (const FMassUnitHandle UnitHandle : SpawnedUnits)
	{
		if (UnitManager->IsUnitValid(UnitHandle))
		{
			ValidUnits.Add(UnitHandle);
		}
	}
	return ValidUnits;
}

int32 AMassUnitSpawner::GetValidSpawnedUnitCount() const
{
	return GetValidSpawnedUnits().Num();
}

int32 AMassUnitSpawner::StartCrowdSimulation()
{
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem ? UnitSubsystem->GetCrowdSystem() : nullptr;
	if (!CrowdSystem || (bSpawnOnAuthorityOnly && GetNetMode() == NM_Client))
	{
		return INDEX_NONE;
	}

	if (CrowdGroupHandle != INDEX_NONE)
	{
		CrowdSystem->UnregisterCrowdGroup(CrowdGroupHandle, false);
		CrowdGroupHandle = INDEX_NONE;
	}
	CrowdGroupHandle = CrowdSystem->RegisterCrowdGroup(
		GetValidSpawnedUnits(),
		GetActorLocation(),
		CrowdConfig,
		bUseNavigation,
		AcceptanceRadius);
	if (CrowdGroupHandle != INDEX_NONE && bEnablePlayerEngagement)
	{
		CrowdSystem->ConfigureCrowdGroupEngagement(CrowdGroupHandle, PlayerEngagementConfig, true);
		CrowdSystem->ForceCrowdGroupUpdate(CrowdGroupHandle);
	}
	return CrowdGroupHandle;
}

void AMassUnitSpawner::StopCrowdSimulation(bool bStopUnits)
{
	if (CrowdGroupHandle == INDEX_NONE)
	{
		return;
	}
	if (UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this))
	{
		if (UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem->GetCrowdSystem())
		{
			CrowdSystem->UnregisterCrowdGroup(CrowdGroupHandle, bStopUnits);
		}
	}
	CrowdGroupHandle = INDEX_NONE;
}

bool AMassUnitSpawner::ForceCrowdUpdate()
{
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem ? UnitSubsystem->GetCrowdSystem() : nullptr;
	return CrowdSystem && CrowdSystem->ForceCrowdGroupUpdate(CrowdGroupHandle);
}

bool AMassUnitSpawner::IsCrowdSimulationActive() const
{
	const UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	const UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem ? UnitSubsystem->GetCrowdSystem() : nullptr;
	return CrowdSystem && CrowdSystem->IsCrowdGroupRegistered(CrowdGroupHandle);
}

bool AMassUnitSpawner::ActivateCrowdAgainstActor(AActor* TargetActor)
{
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem ? UnitSubsystem->GetCrowdSystem() : nullptr;
	return CrowdSystem && CrowdSystem->ActivateCrowdGroupForActor(CrowdGroupHandle, TargetActor);
}

bool AMassUnitSpawner::DeactivateCrowdEngagement(bool bReturnToWander)
{
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem ? UnitSubsystem->GetCrowdSystem() : nullptr;
	return CrowdSystem && CrowdSystem->DeactivateCrowdGroupEngagement(CrowdGroupHandle, bReturnToWander);
}

bool AMassUnitSpawner::NotifySpawnedUnitInteracted(FMassUnitHandle UnitHandle, AActor* InteractingActor)
{
	if (!SpawnedUnits.Contains(UnitHandle))
	{
		return false;
	}
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem ? UnitSubsystem->GetCrowdSystem() : nullptr;
	return CrowdSystem && CrowdSystem->NotifyUnitInteracted(UnitHandle, InteractingActor);
}

bool AMassUnitSpawner::DamageSpawnedUnitAndActivate(
	FMassUnitHandle UnitHandle,
	float Damage,
	AActor* DamageInstigator)
{
	if (!SpawnedUnits.Contains(UnitHandle))
	{
		return false;
	}
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem ? UnitSubsystem->GetCrowdSystem() : nullptr;
	return CrowdSystem && CrowdSystem->DamageUnitAndActivate(UnitHandle, Damage, DamageInstigator);
}

FMassUnitHandle AMassUnitSpawner::FindClosestSpawnedUnit(
	FVector WorldLocation,
	float MaxDistance,
	bool bUse3DDistance) const
{
	const UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	const UMassUnitEntityManager* UnitManager = UnitSubsystem ? UnitSubsystem->GetUnitManager() : nullptr;
	if (!UnitManager || MaxDistance < 0.0f)
	{
		return {};
	}

	float BestDistanceSquared = FMath::Square(MaxDistance);
	FMassUnitHandle BestUnit;
	for (const FMassUnitHandle UnitHandle : SpawnedUnits)
	{
		FTransform Transform;
		FMassUnitStateFragment State;
		if (!UnitManager->GetUnitTransform(UnitHandle, Transform)
			|| !UnitManager->GetUnitState(UnitHandle, State)
			|| State.CurrentState == EMassUnitState::Dead)
		{
			continue;
		}
		const FVector Delta = Transform.GetLocation() - WorldLocation;
		const float DistanceSquared = bUse3DDistance ? Delta.SizeSquared() : Delta.SizeSquared2D();
		if (DistanceSquared <= BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestUnit = UnitHandle;
		}
	}
	return BestUnit;
}

FVector AMassUnitSpawner::CalculateLocalGridOffset(int32 UnitIndex, int32 TotalUnits) const
{
	const int32 SafeColumns = FMath::Max(1, Columns);
	const int32 RowCount = FMath::Max(1, FMath::DivideAndRoundUp(TotalUnits, SafeColumns));
	const int32 RowIndex = UnitIndex / SafeColumns;
	const int32 ColumnIndex = UnitIndex % SafeColumns;
	const int32 UnitsInRow = FMath::Min(SafeColumns, TotalUnits - (RowIndex * SafeColumns));
	const float CenteredRow = static_cast<float>(RowIndex) - (static_cast<float>(RowCount - 1) * 0.5f);
	const float CenteredColumn = static_cast<float>(ColumnIndex) - (static_cast<float>(UnitsInRow - 1) * 0.5f);
	return FVector(CenteredRow * UnitSpacing.X, CenteredColumn * UnitSpacing.Y, SpawnHeight);
}

bool AMassUnitSpawner::CommandUnit(FMassUnitHandle UnitHandle, const FVector& Destination) const
{
	UMassUnitSubsystem* UnitSubsystem = UMassUnitSubsystem::Get(this);
	if (!UnitSubsystem)
	{
		return false;
	}
	if (bUseNavigation)
	{
		UMassUnitNavigationSystem* NavigationSystem = UnitSubsystem->GetNavigationSystem();
		return NavigationSystem && NavigationSystem->RequestPath(UnitHandle, Destination, AcceptanceRadius);
	}
	UMassUnitEntityManager* UnitManager = UnitSubsystem->GetUnitManager();
	return UnitManager && UnitManager->SetUnitDestination(UnitHandle, Destination, AcceptanceRadius);
}

void AMassUnitSpawner::DrawSpawnDebug(const FVector& Location) const
{
#if ENABLE_DRAW_DEBUG
	if (bEnableVisualDebug && GetWorld())
	{
		DrawDebugBox(GetWorld(), Location, FVector(45.0f), FColor::Cyan, false, DebugDuration, 0, 2.0f);
	}
#endif
}

void AMassUnitSpawner::DrawCommandDebug(const FVector& Start, const FVector& Destination) const
{
#if ENABLE_DRAW_DEBUG
	if (bEnableVisualDebug && GetWorld())
	{
		DrawDebugDirectionalArrow(GetWorld(), Start, Destination, 75.0f, FColor::Green, false, DebugDuration, 0, 2.0f);
	}
#endif
}
