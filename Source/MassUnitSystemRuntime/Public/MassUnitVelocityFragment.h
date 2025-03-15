// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * This file includes velocity fragment used throughout the plugin.
 * It serves as a bridge to the actual MassEntity module's fragments.
 */

#include "MassUnitVelocityFragment.generated.h"

/**
 * Velocity fragment for Mass units
 */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVelocityFragment
{
    GENERATED_BODY()

    /** Current velocity */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    FVector Value = FVector::ZeroVector;

    /** Maximum speed */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    float MaxSpeed = 600.0f;
};
