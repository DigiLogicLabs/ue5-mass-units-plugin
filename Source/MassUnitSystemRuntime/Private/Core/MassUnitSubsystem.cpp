// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Core/MassUnitSubsystem.h"

#include "Config/MassUnitSystemSettings.h"
#include "Core/MassUnitSystemRuntime.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Entity/MassUnitEntityManager.h"
#include "Gameplay/GASUnitIntegration.h"
#include "Gameplay/MassUnitBehaviorIntegration.h"
#include "Gameplay/UnitGameplayEventSystem.h"
#include "MassEntitySubsystem.h"
#include "Navigation/FormationSystem.h"
#include "Navigation/MassUnitNavigationSystem.h"
#include "Visual/NiagaraUnitSystem.h"
#include "Visual/UnitMeshPool.h"

void UMassUnitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	EntitySubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
	if (!EntitySubsystem || !GetWorld())
	{
		UE_LOG(LogMassUnitSystem, Error, TEXT("Mass Unit subsystem could not resolve its world Mass entity subsystem"));
		return;
	}

	UnitManager = NewObject<UMassUnitEntityManager>(this);
	UnitManager->Initialize(EntitySubsystem);

	FormationSystem = NewObject<UFormationSystem>(this);
	FormationSystem->Initialize(EntitySubsystem);

	NavigationSystem = NewObject<UMassUnitNavigationSystem>(this);
	NavigationSystem->Initialize(GetWorld(), EntitySubsystem);

	NiagaraSystem = NewObject<UNiagaraUnitSystem>(this);
	NiagaraSystem->Initialize(GetWorld(), EntitySubsystem);

	MeshPool = NewObject<UUnitMeshPool>(this);
	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	MeshPool->Initialize(GetWorld(), EntitySubsystem, Settings ? Settings->MaxSkeletalMeshUnits : 100);

	GASIntegration = NewObject<UGASUnitIntegration>(this);
	GASIntegration->Initialize(EntitySubsystem);

	BehaviorIntegration = NewObject<UMassUnitBehaviorIntegration>(this);
	BehaviorIntegration->Initialize(GASIntegration);

	GameplayEventSystem = NewObject<UUnitGameplayEventSystem>(this);
	GameplayEventSystem->Initialize();

	UE_LOG(LogMassUnitSystem, Log, TEXT("Mass Unit subsystem initialized for %s"), *GetWorld()->GetName());
}

void UMassUnitSubsystem::Deinitialize()
{
	if (GameplayEventSystem)
	{
		GameplayEventSystem->Deinitialize();
	}
	if (BehaviorIntegration)
	{
		BehaviorIntegration->Deinitialize();
	}
	if (GASIntegration)
	{
		GASIntegration->Deinitialize();
	}
	if (MeshPool)
	{
		MeshPool->Deinitialize();
	}
	if (NiagaraSystem)
	{
		NiagaraSystem->Deinitialize();
	}
	if (NavigationSystem)
	{
		NavigationSystem->Deinitialize();
	}
	if (FormationSystem)
	{
		FormationSystem->Deinitialize();
	}
	if (UnitManager)
	{
		UnitManager->Deinitialize();
	}

	GameplayEventSystem = nullptr;
	BehaviorIntegration = nullptr;
	GASIntegration = nullptr;
	MeshPool = nullptr;
	NiagaraSystem = nullptr;
	NavigationSystem = nullptr;
	FormationSystem = nullptr;
	UnitManager = nullptr;
	EntitySubsystem = nullptr;

	Super::Deinitialize();
}

void UMassUnitSubsystem::Tick(float DeltaTime)
{
	if (!UnitManager)
	{
		return;
	}
	UnitManager->PruneInvalidUnits();

	if (FormationSystem)
	{
		FormationSystem->Tick(DeltaTime);
	}
	if (NavigationSystem)
	{
		NavigationSystem->ProcessPathRequests();
	}
	if (BehaviorIntegration)
	{
		BehaviorIntegration->Tick(DeltaTime);
	}

	const TArray<FMassUnitEntityHandle> Units = UnitManager->GetAllUnitsInternal();
	if (MeshPool)
	{
		MeshPool->UpdateUnitMeshes(Units);
	}
	if (NiagaraSystem)
	{
		NiagaraSystem->UpdateUnitVisuals(Units);
	}
}

TStatId UMassUnitSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMassUnitSubsystem, STATGROUP_Tickables);
}

bool UMassUnitSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::PIE
		|| WorldType == EWorldType::GamePreview;
}

UMassUnitSubsystem* UMassUnitSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject || !GEngine)
	{
		return nullptr;
	}
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		return World->GetSubsystem<UMassUnitSubsystem>();
	}
	return nullptr;
}
