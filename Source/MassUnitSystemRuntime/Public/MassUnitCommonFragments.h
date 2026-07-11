// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassUnitCommonFragments.generated.h"

USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVelocityFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector Value = FVector::ZeroVector;
};

USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitForceFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector Value = FVector::ZeroVector;
};

USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitLookAtFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector Direction = FVector::ForwardVector;
};
