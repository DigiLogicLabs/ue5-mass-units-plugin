// Copyright Digi Logic Labs LLC. All Rights Reserved.

#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Core/MassUnitGameplayTags.h"
#include "Core/MassUnitSubsystem.h"
#include "Engine/World.h"
#include "Entity/MassUnitCombatProcessor.h"
#include "Entity/MassUnitEntityManager.h"
#include "Entity/MassUnitFragments.h"
#include "Entity/MassUnitMovementProcessor.h"
#include "Entity/UnitTemplate.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Navigation/FormationSystem.h"
#include "Navigation/MassUnitNavigationSystem.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMassUnitNativeLifecycleTest,
	"MassUnitSystem.Core.NativeMassLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMassUnitNativeLifecycleTest::RunTest(const FString& Parameters)
{
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
	return true;
}

#endif // WITH_AUTOMATION_TESTS
