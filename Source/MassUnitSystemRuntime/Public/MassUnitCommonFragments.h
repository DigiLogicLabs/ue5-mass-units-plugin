// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Include MassEntity types
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"

// Include the engine's MassCommonFragments.h to get access to the transform fragment
#include "MassCommonFragments.h"

/**
 * This file includes common Mass fragments used throughout the plugin.
 * It serves as a bridge to the actual MassEntity module's fragments.
 */

#include "MassUnitCommonFragments.generated.h"

/**
 * Velocity fragment for Mass units
 */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVelocityFragment // Renamed to avoid conflict
{
    GENERATED_BODY()

    /** Current velocity */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    FVector Value = FVector::ZeroVector;

    /** Maximum speed */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    float MaxSpeed = 600.0f;
};
