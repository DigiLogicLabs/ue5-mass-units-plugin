// Copyright Digi Logic Labs LLC. All Rights Reserved.

#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Templates/UnrealTemplate.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Config/MassUnitSystemSettings.h"
#include "Core/MassUnitGameplayTags.h"
#include "Core/MassUnitSubsystem.h"
#include "Engine/World.h"
#include "Entity/MassUnitCombatProcessor.h"
#include "Entity/MassUnitEntityManager.h"
#include "Entity/MassUnitFragments.h"
#include "Entity/MassUnitMovementProcessor.h"
#include "Entity/MassUnitSpawner.h"
#include "Entity/UnitTemplate.h"
#include "Gameplay/MassUnitCrowdSystem.h"
#include "Kismet/GameplayStatics.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Navigation/FormationSystem.h"
#include "Navigation/MassUnitNavigationSystem.h"
#include "Tests/AutomationCommon.h"
#include "UObject/UObjectIterator.h"
#include "Visual/NiagaraUnitSystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMassUnitNativeLifecycleTest,
	"MassUnitSystem.Core.NativeMassLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMassUnitNativeLifecycleTest::RunTest(const FString& Parameters)
{
	UMassUnitSystemSettings* MutableSettings = GetMutableDefault<UMassUnitSystemSettings>();
	TGuardValue<float> VisualUpdateIntervalGuard(MutableSettings->VisualUpdateInterval, 0.0f);

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A transient game world can be created"), TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	UMassUnitSubsystem* UnitSubsystem = World ? World->GetSubsystem<UMassUnitSubsystem>() : nullptr;
	if (!TestNotNull(TEXT("Mass Unit world subsystem initializes automatically"), UnitSubsystem))
	{
		return false;
	}

	UMassUnitEntityManager* UnitManager = UnitSubsystem->GetUnitManager();
	UMassEntitySubsystem* MassSubsystem = UnitSubsystem->GetEntitySubsystem();
	if (!TestNotNull(TEXT("Unit manager is available"), UnitManager)
		|| !TestNotNull(TEXT("Native Mass subsystem is available"), MassSubsystem))
	{
		return false;
	}
	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();

	UUnitTemplate* Template = NewObject<UUnitTemplate>(GetTransientPackage());
	Template->TeamID = 1;
	Template->BaseHealth = 100;
	Template->BaseDamage = 20;
	Template->MoveSpeed = 300.0f;
	Template->AttackRange = 200.0f;
	Template->AttackCooldown = 0.0f;
	Template->DefaultAbilities.Add(UE::MassUnitSystem::Tags::AnimationAttack());

	const FMassUnitHandle UnitA = UnitManager->CreateUnitFromTemplate(Template, FTransform(FVector::ZeroVector));
	Template->TeamID = 2;
	const FMassUnitHandle UnitB = UnitManager->CreateUnitFromTemplate(Template, FTransform(FVector(100.0f, 0.0f, 0.0f)));
	TestTrue(TEXT("First native Mass unit is valid"), UnitManager->IsUnitValid(UnitA));
	TestTrue(TEXT("Second native Mass unit is valid"), UnitManager->IsUnitValid(UnitB));
	TestNotEqual(TEXT("Units receive distinct native handles"), UnitA.EntityHandle.Index, UnitB.EntityHandle.Index);
	TestEqual(TEXT("Unit manager tracks both units"), UnitManager->GetUnitCount(), 2);
	const FMassUnitStateFragment* StateA = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(UnitA.EntityHandle.ToMassEntityHandle());
	const FMassUnitAbilityFragment* AbilityA = EntityManager.GetFragmentDataPtr<FMassUnitAbilityFragment>(UnitA.EntityHandle.ToMassEntityHandle());
	const FMassUnitVisualFragment* VisualA = EntityManager.GetFragmentDataPtr<FMassUnitVisualFragment>(UnitA.EntityHandle.ToMassEntityHandle());
	const FMassUnitFormationFragment* FormationA = EntityManager.GetFragmentDataPtr<FMassUnitFormationFragment>(UnitA.EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("Template class and behavior tags are copied into native fragments"), StateA && StateA->UnitClass == Template->UnitClass && StateA->DefaultBehavior == Template->DefaultBehavior);
	TestTrue(TEXT("Template ability tags are copied into the ability fragment"), AbilityA && AbilityA->DefaultAbilityTags == Template->DefaultAbilities);
	TestTrue(TEXT("Template animation metadata is copied into the visual fragment"), VisualA && VisualA->AnimationTags == Template->AnimationTags);
	TestTrue(TEXT("Template formation preference is copied into the formation fragment"), FormationA && FormationA->DefaultFormation == Template->DefaultFormation);

	FTransform TransformA;
	FTransform TransformB;
	TestTrue(TEXT("First unit transform is readable"), UnitManager->GetUnitTransform(UnitA, TransformA));
	TestTrue(TEXT("Second unit transform is readable"), UnitManager->GetUnitTransform(UnitB, TransformB));
	TestTrue(TEXT("Per-entity transform fragments remain independent"), !TransformA.GetLocation().Equals(TransformB.GetLocation()));

	TestTrue(TEXT("A hostile target can be assigned"), UnitManager->SetUnitTarget(UnitA, UnitB));
	UMassUnitCombatProcessor* CombatProcessor = NewObject<UMassUnitCombatProcessor>(GetTransientPackage());
	CombatProcessor->CallInitialize(World, EntityManager.AsShared());
	FMassExecutionContext CombatContext(EntityManager, 0.1f);
	CombatContext.SetExecutionType(EMassExecutionContextType::Processor);
	CombatProcessor->CallExecute(EntityManager, CombatContext);
	FMassUnitStateFragment StateB;
	TestTrue(TEXT("Target state remains readable after combat"), UnitManager->GetUnitState(UnitB, StateB));
	TestTrue(TEXT("Combat processor applies configured damage"), StateB.Health < StateB.MaxHealth);

	UnitManager->ClearUnitTarget(UnitA);
	TestTrue(TEXT("Direct path request queues successfully"), UnitSubsystem->GetNavigationSystem()->RequestPath(UnitA, FVector(400.0f, 0.0f, 0.0f), 10.0f));
	TestTrue(TEXT("A newer path request supersedes the queued request"), UnitSubsystem->GetNavigationSystem()->RequestPath(UnitA, FVector(500.0f, 0.0f, 0.0f), 10.0f));
	TestEqual(TEXT("Only the newest request remains queued for a unit"), UnitSubsystem->GetNavigationSystem()->GetQueuedRequestCount(), 1);
	UnitSubsystem->GetNavigationSystem()->ProcessPathRequests();
	const FMassUnitNavigationFragment* Navigation = EntityManager.GetFragmentDataPtr<FMassUnitNavigationFragment>(UnitA.EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("A world without nav data receives the newest direct-path fallback"), Navigation && Navigation->bPathValid && Navigation->PathPoints.Num() == 1 && Navigation->PathPoints[0].Equals(FVector(500.0f, 0.0f, 0.0f)));

	UMassUnitMovementProcessor* MovementProcessor = NewObject<UMassUnitMovementProcessor>(GetTransientPackage());
	MovementProcessor->CallInitialize(World, EntityManager.AsShared());
	for (int32 Step = 0; Step < 5; ++Step)
	{
		FMassExecutionContext MovementContext(EntityManager, 0.1f);
		MovementContext.SetExecutionType(EMassExecutionContextType::Processor);
		MovementProcessor->CallExecute(EntityManager, MovementContext);
	}
	FTransform MovedTransformA;
	FTransform UnmovedTransformB;
	UnitManager->GetUnitTransform(UnitA, MovedTransformA);
	UnitManager->GetUnitTransform(UnitB, UnmovedTransformB);
	TestTrue(TEXT("Requested unit moves toward its destination"), MovedTransformA.GetLocation().X > TransformA.GetLocation().X);
	TestTrue(TEXT("Unrequested unit does not share movement fragments"), UnmovedTransformB.GetLocation().Equals(TransformB.GetLocation()));
	TestTrue(TEXT("A path can be queued before explicit cancellation"),
		UnitSubsystem->GetNavigationSystem()->RequestPath(UnitA, FVector(700.0f, 0.0f, 0.0f), 10.0f));
	TestEqual(TEXT("Cancellation test has one queued request"), UnitSubsystem->GetNavigationSystem()->GetQueuedRequestCount(), 1);
	TestTrue(TEXT("Queued navigation can be cancelled safely"), UnitSubsystem->GetNavigationSystem()->CancelPath(UnitA));
	TestEqual(TEXT("Cancellation removes queued and in-flight work"), UnitSubsystem->GetNavigationSystem()->GetQueuedRequestCount(), 0);
	Navigation = EntityManager.GetFragmentDataPtr<FMassUnitNavigationFragment>(UnitA.EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("Cancellation clears the unit navigation fragment"),
		Navigation && !Navigation->bPathRequested && !Navigation->bPathValid && Navigation->PathPoints.IsEmpty());

	UFormationSystem* FormationSystem = UnitSubsystem->GetFormationSystem();
	const int32 FormationHandle = FormationSystem->CreateFormation(FVector::ZeroVector, FRotator::ZeroRotator, TEXT("Infantry"));
	TestFalse(TEXT("Unknown formation shapes are rejected"), FormationSystem->SetFormationShape(FormationHandle, TEXT("Unknown")));
	TestTrue(TEXT("Column formation shape is supported"), FormationSystem->SetFormationShape(FormationHandle, TEXT("Column")));
	TestTrue(TEXT("Unit can join a formation"), FormationSystem->AddUnitToFormation(UnitA, FormationHandle));
	TestEqual(TEXT("Formation contains exactly one unit"), FormationSystem->GetEntitiesInFormation(FormationHandle).Num(), 1);
	FormationSystem->Tick(0.0f);
	const FMassUnitTargetFragment* FormationTarget = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(UnitA.EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("World origin remains a valid formation target"), FormationTarget && FormationTarget->bHasTargetLocation && FormationTarget->TargetLocation.IsNearlyZero());
	TestTrue(TEXT("Unit can leave a formation"), FormationSystem->RemoveUnitFromFormation(UnitA));

	UnitManager->DestroyUnit(UnitA);
	TestFalse(TEXT("Destroyed native handle is rejected"), UnitManager->IsUnitValid(UnitA));
	TestEqual(TEXT("Unit manager removes destroyed units"), UnitManager->GetUnitCount(), 1);
	EntityManager.DestroyEntity(UnitB.EntityHandle.ToMassEntityHandle());
	UnitManager->PruneInvalidUnits();
	TestEqual(TEXT("Externally destroyed native entities are pruned from manager indexes"), UnitManager->GetUnitCount(), 0);
	TestEqual(TEXT("Team queries exclude externally destroyed entities"), UnitManager->GetUnitsByTeam(2).Num(), 0);

	const FMassUnitHandle DefaultUnit = UnitManager->CreateDefaultUnit(FTransform(FVector(0.0f, 0.0f, 50.0f)));
	FMassUnitStateFragment DefaultState;
	TestTrue(TEXT("Asset-free default unit creation produces a valid native entity"), UnitManager->IsUnitValid(DefaultUnit));
	TestTrue(TEXT("Asset-free default unit uses playable movement and health defaults"),
		UnitManager->GetUnitState(DefaultUnit, DefaultState) && DefaultState.Health > 0.0f && DefaultState.MoveSpeed > 0.0f);
	UnitManager->DestroyUnit(DefaultUnit);

	AMassUnitSpawner* Spawner = World->SpawnActorDeferred<AMassUnitSpawner>(
		AMassUnitSpawner::StaticClass(),
		FTransform::Identity,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!TestNotNull(TEXT("Quick-start spawner can be created in a game world"), Spawner))
	{
		return false;
	}
	Spawner->bSpawnOnBeginPlay = false;
	Spawner->bMoveOnBeginPlay = false;
	Spawner->bSpawnOnAuthorityOnly = false;
	Spawner->UnitCount = 6;
	Spawner->Columns = 3;
	UGameplayStatics::FinishSpawningActor(Spawner, FTransform::Identity);

	const TArray<FMassUnitHandle> SpawnedUnits = Spawner->SpawnUnits();
	TestEqual(TEXT("Quick-start spawner creates the requested native unit count"), SpawnedUnits.Num(), 6);
	TestEqual(TEXT("Quick-start spawner tracks only valid owned units"), Spawner->GetValidSpawnedUnitCount(), 6);
	TestEqual(TEXT("Unit manager sees every quick-start unit"), UnitManager->GetUnitCount(), 6);
	if (SpawnedUnits.IsEmpty())
	{
		return false;
	}

	FTransform FirstSpawnerTransform;
	TestTrue(TEXT("Quick-start unit transform is readable"), UnitManager->GetUnitTransform(SpawnedUnits[0], FirstSpawnerTransform));
	TestEqual(TEXT("Quick-start offset command reaches every unit"), Spawner->MoveSpawnedUnitsByOffset(FVector(600.0f, 0.0f, 0.0f)), 6);
	const FMassUnitNavigationFragment* SpawnerNavigation = EntityManager.GetFragmentDataPtr<FMassUnitNavigationFragment>(SpawnedUnits[0].EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("Quick-start offset command preserves each unit's own start location"),
		SpawnerNavigation && SpawnerNavigation->PathPoints.Num() == 1
		&& SpawnerNavigation->PathPoints[0].Equals(FirstSpawnerTransform.GetLocation() + FVector(600.0f, 0.0f, 0.0f)));

	UMassUnitCrowdSystem* CrowdSystem = UnitSubsystem->GetCrowdSystem();
	if (!TestNotNull(TEXT("World crowd service is available"), CrowdSystem))
	{
		return false;
	}
	Spawner->bEnableCrowdSimulation = true;
	Spawner->CrowdConfig.WanderRadius = 500.0f;
	Spawner->CrowdConfig.MinWanderDistance = 100.0f;
	Spawner->CrowdConfig.MinIdleTime = 0.0f;
	Spawner->CrowdConfig.MaxIdleTime = 0.0f;
	Spawner->CrowdConfig.MaxSimulationDistance = 0.0f;
	Spawner->CrowdConfig.bEnableInteractions = false;
	Spawner->CrowdConfig.RandomSeed = 2468;
	const int32 CrowdGroupHandle = Spawner->StartCrowdSimulation();
	TestTrue(TEXT("Spawner registers a continuous crowd group"), CrowdGroupHandle != INDEX_NONE);
	TestTrue(TEXT("Spawner reports active crowd simulation"), Spawner->IsCrowdSimulationActive());
	TestEqual(TEXT("Crowd group owns every spawned unit"), CrowdSystem->GetCrowdGroupUnitCount(CrowdGroupHandle), SpawnedUnits.Num());

	const FMassUnitCrowdFragment* FirstCrowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(SpawnedUnits[0].EntityHandle.ToMassEntityHandle());
	const FMassUnitTargetFragment* FirstCrowdTarget = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(SpawnedUnits[0].EntityHandle.ToMassEntityHandle());
	const FMassUnitTargetFragment* SecondCrowdTarget = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(SpawnedUnits[1].EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("Crowd registration writes lightweight per-unit state"),
		FirstCrowd && FirstCrowd->bEnabled && FirstCrowd->GroupHandle == CrowdGroupHandle);
	TestTrue(TEXT("Crowd registration immediately assigns a random destination"),
		FirstCrowdTarget && FirstCrowdTarget->bHasTargetLocation);
	TestTrue(TEXT("Random destinations remain inside the configured group radius"),
		FirstCrowdTarget && FVector::DistSquared2D(FirstCrowdTarget->TargetLocation, Spawner->GetActorLocation())
			<= FMath::Square(Spawner->CrowdConfig.WanderRadius));
	TestTrue(TEXT("Deterministic per-unit streams avoid one shared crowd destination"),
		FirstCrowdTarget && SecondCrowdTarget
		&& !FirstCrowdTarget->TargetLocation.Equals(SecondCrowdTarget->TargetLocation));
	TestTrue(TEXT("Crowd diagnostics report assigned destinations"),
		CrowdSystem->GetCrowdStats().DestinationsAssigned > 0);
	TestEqual(TEXT("Crowd diagnostics are populated immediately after registration"),
		CrowdSystem->GetCrowdStats().RegisteredUnits,
		SpawnedUnits.Num());

	FTransform CrowdStartTransform;
	FTransform CrowdMovedTransform;
	TestTrue(TEXT("Crowd movement baseline transform is readable"),
		UnitManager->GetUnitTransform(SpawnedUnits[0], CrowdStartTransform));
	for (int32 Step = 0; Step < 3; ++Step)
	{
		FMassExecutionContext CrowdMovementContext(EntityManager, 0.1f);
		CrowdMovementContext.SetExecutionType(EMassExecutionContextType::Processor);
		MovementProcessor->CallExecute(EntityManager, CrowdMovementContext);
	}
	TestTrue(TEXT("Crowd movement result transform is readable"),
		UnitManager->GetUnitTransform(SpawnedUnits[0], CrowdMovedTransform));
	TestTrue(TEXT("An active crowd destination produces smooth Mass movement"),
		FVector::DistSquared2D(CrowdStartTransform.GetLocation(), CrowdMovedTransform.GetLocation()) > 1.0f);

	TestTrue(TEXT("Crowd group can be paused"), CrowdSystem->SetCrowdGroupPaused(CrowdGroupHandle, true, true));
	FirstCrowdTarget = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(SpawnedUnits[0].EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("Pausing a crowd clears active movement"), FirstCrowdTarget && !FirstCrowdTarget->HasTarget());
	TestTrue(TEXT("Crowd group can be resumed"), CrowdSystem->SetCrowdGroupPaused(CrowdGroupHandle, false, true));
	FirstCrowdTarget = EntityManager.GetFragmentDataPtr<FMassUnitTargetFragment>(SpawnedUnits[0].EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("Resuming a crowd assigns fresh work"), FirstCrowdTarget && FirstCrowdTarget->bHasTargetLocation);
	Spawner->StopCrowdSimulation(false);
	TestFalse(TEXT("Spawner unregisters its crowd group without destroying units"), Spawner->IsCrowdSimulationActive());
	FirstCrowd = EntityManager.GetFragmentDataPtr<FMassUnitCrowdFragment>(SpawnedUnits[0].EntityHandle.ToMassEntityHandle());
	TestTrue(TEXT("Unregistering restores non-crowd unit state"), FirstCrowd && !FirstCrowd->bEnabled);

	FMassUnitCrowdConfig InteractionConfig;
	InteractionConfig.WanderRadius = 500.0f;
	InteractionConfig.bEnableSeparation = false;
	InteractionConfig.bEnableInteractions = true;
	InteractionConfig.InteractionChance = 1.0f;
	InteractionConfig.InteractionRadius = 500.0f;
	InteractionConfig.MinInteractionTime = 1.0f;
	InteractionConfig.MaxInteractionTime = 1.0f;
	InteractionConfig.MaxSimulationDistance = 0.0f;
	const int32 InteractionGroupHandle = CrowdSystem->RegisterCrowdGroup(
		SpawnedUnits,
		Spawner->GetActorLocation(),
		InteractionConfig,
		false,
		25.0f);
	TestTrue(TEXT("Crowd interaction test group registers"), InteractionGroupHandle != INDEX_NONE);
	TestTrue(TEXT("Nearby crowd members can begin lightweight paired interactions"),
		CrowdSystem->GetCrowdStats().InteractionsStarted > 0);
	bool bFoundInteractingUnit = false;
	for (const FMassUnitHandle& UnitHandle : SpawnedUnits)
	{
		const FMassUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMassUnitStateFragment>(UnitHandle.EntityHandle.ToMassEntityHandle());
		bFoundInteractingUnit |= State && State->CurrentState == EMassUnitState::Interacting;
	}
	TestTrue(TEXT("Crowd interactions write the lightweight interaction state"), bFoundInteractingUnit);
	TestTrue(TEXT("Interaction group unregisters cleanly"), CrowdSystem->UnregisterCrowdGroup(InteractionGroupHandle, true));

	UNiagaraUnitSystem* VisualSystem = UnitSubsystem->GetNiagaraSystem();
	if (!TestNotNull(TEXT("Quick-start visual service is available"), VisualSystem))
	{
		return false;
	}
	VisualSystem->UpdateUnitVisualsByHandles(SpawnedUnits);
	TestTrue(TEXT("Quick-start units have either configured Niagara or asset-free instanced rendering"),
		VisualSystem->IsUsingNiagara() || VisualSystem->GetInstancedMeshInstanceCount() == SpawnedUnits.Num());

	if (!VisualSystem->IsUsingNiagara())
	{
		UInstancedStaticMeshComponent* FallbackComponent = nullptr;
		for (TObjectIterator<UInstancedStaticMeshComponent> It; It; ++It)
		{
			if (It->GetWorld() == World && It->GetInstanceCount() == SpawnedUnits.Num())
			{
				FallbackComponent = *It;
				break;
			}
		}
		if (!TestNotNull(TEXT("Asset-free fallback creates an instanced mesh component"), FallbackComponent))
		{
			return false;
		}
		TestFalse(TEXT("Moving fallback units avoid HISM cluster-tree rebuilds"),
			FallbackComponent->IsA<UHierarchicalInstancedStaticMeshComponent>());

		FTransform BeforeVisualMove;
		TestTrue(TEXT("Fallback instance transform is readable"),
			FallbackComponent->GetInstanceTransform(0, BeforeVisualMove, true));
		FTransform UpdatedUnitTransform;
		UnitManager->GetUnitTransform(SpawnedUnits[0], UpdatedUnitTransform);
		UpdatedUnitTransform.AddToTranslation(FVector(75.0f, 25.0f, 0.0f));
		const int32 TopologyRevisionBeforeMove = VisualSystem->GetInstancedMeshTopologyRevision();
		TestTrue(TEXT("Quick-start unit transform can be updated for rendering"),
			UnitManager->SetUnitTransform(SpawnedUnits[0], UpdatedUnitTransform));
		VisualSystem->UpdateUnitVisualsByHandles(SpawnedUnits);

		FTransform AfterVisualMove;
		TestTrue(TEXT("Fallback instance remains readable after an in-place update"),
			FallbackComponent->GetInstanceTransform(0, AfterVisualMove, true));
		TestTrue(TEXT("Fallback instance receives the updated unit transform"),
			AfterVisualMove.GetLocation().Equals(UpdatedUnitTransform.GetLocation()));
		TestEqual(TEXT("Movement does not rebuild instanced-mesh topology"),
			VisualSystem->GetInstancedMeshTopologyRevision(), TopologyRevisionBeforeMove);
		TestEqual(TEXT("Movement preserves the fallback instance count"),
			VisualSystem->GetInstancedMeshInstanceCount(), SpawnedUnits.Num());
	}

	Spawner->DestroySpawnedUnits();
	TestEqual(TEXT("Quick-start cleanup destroys only its owned Mass units"), UnitManager->GetUnitCount(), 0);
	VisualSystem->UpdateUnitVisualsByHandles(Spawner->GetValidSpawnedUnits());
	TestTrue(TEXT("Quick-start cleanup removes instanced fallback slots"),
		VisualSystem->IsUsingNiagara() || VisualSystem->GetInstancedMeshInstanceCount() == 0);
	World->DestroyActor(Spawner);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
