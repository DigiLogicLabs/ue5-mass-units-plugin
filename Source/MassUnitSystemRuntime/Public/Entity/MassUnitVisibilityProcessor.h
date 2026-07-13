// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "MassUnitVisibilityProcessor.generated.h"

/** Updates plugin-specific distance LOD and representation intent. */
UCLASS()
class MASSUNITSYSTEMRUNTIME_API UMassUnitVisibilityProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassUnitVisibilityProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
