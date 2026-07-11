// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Entity/MassUnitVisibilityProcessor.h"

#include "Config/MassUnitSystemSettings.h"
#include "Engine/World.h"
#include "Entity/MassUnitFragments.h"
#include "GameFramework/PlayerController.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"

UMassUnitVisibilityProcessor::UMassUnitVisibilityProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = FName(TEXT("MassUnitSystem.Visibility"));
	ExecutionOrder.ExecuteAfter.Add(FName(TEXT("MassUnitSystem.Combat")));
}

void UMassUnitVisibilityProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassUnitTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassUnitVisualFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassUnitLODFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassUnitVisualizationLODFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassUnitVisibilityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	FVector ViewLocation = FVector::ZeroVector;
	bool bHasView = false;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* Controller = It->Get(); Controller && Controller->IsLocalController())
		{
			FRotator ViewRotation;
			Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
			bHasView = true;
			break;
		}
	}
	if (!bHasView)
	{
		return;
	}

	const UMassUnitSystemSettings* Settings = GetDefault<UMassUnitSystemSettings>();
	const TArray<float> Thresholds = Settings ? Settings->LODDistanceThresholds : TArray<float>{500.0f, 1500.0f, 3000.0f, 6000.0f};
	const float SkeletalDistance = Settings ? Settings->SkeletalMeshDistance : 300.0f;
	const float MaxVisibleDistance = Settings ? Settings->MaxVisibleDistance : 10000.0f;

	EntityQuery.ForEachEntityChunk(Context, [ViewLocation, Thresholds, SkeletalDistance, MaxVisibleDistance](FMassExecutionContext& ChunkContext)
	{
		const TConstArrayView<FMassUnitTransformFragment> Transforms = ChunkContext.GetFragmentView<FMassUnitTransformFragment>();
		TArrayView<FMassUnitVisualFragment> Visuals = ChunkContext.GetMutableFragmentView<FMassUnitVisualFragment>();
		TArrayView<FMassUnitLODFragment> LODs = ChunkContext.GetMutableFragmentView<FMassUnitLODFragment>();
		TArrayView<FMassUnitVisualizationLODFragment> VisualizationLODs = ChunkContext.GetMutableFragmentView<FMassUnitVisualizationLODFragment>();

		for (FMassExecutionContext::FEntityIterator It = ChunkContext.CreateEntityIterator(); It; ++It)
		{
			const float DistanceSquared = FVector::DistSquared(Transforms[It].GetTransform().GetLocation(), ViewLocation);
			int32 LODLevel = Thresholds.Num();
			for (int32 Index = 0; Index < Thresholds.Num(); ++Index)
			{
				if (DistanceSquared <= FMath::Square(FMath::Max(0.0f, Thresholds[Index])))
				{
					LODLevel = Index;
					break;
				}
			}

			FMassUnitVisualFragment& Visual = Visuals[It];
			Visual.LODLevel = LODLevel;
			Visual.bIsVisible = MaxVisibleDistance <= 0.0f || DistanceSquared <= FMath::Square(MaxVisibleDistance);
			Visual.bUseSkeletalMesh = Visual.bIsVisible && Visual.SkeletalMesh != nullptr
				&& DistanceSquared <= FMath::Square(SkeletalDistance);
			LODs[It].Level = LODLevel;
			VisualizationLODs[It].LODLevel = LODLevel;
		}
	});
}
