// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "MassUnitFragments.generated.h"

/** Enum for unit states */
UENUM(BlueprintType)
enum class EMassUnitState : uint8
{
    Idle,
    Moving,
    Attacking,
    Defending,
    Dead,
    Interacting,
    Stunned,
    Custom
};

/**
 * Fragment for unit state information
 */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitStateFragment
{
    GENERATED_BODY()

    /** Current state of the unit */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    EMassUnitState CurrentState = EMassUnitState::Idle;

    /** Time spent in current state */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    float StateTime = 0.0f;

    /** Unit type tag */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    FGameplayTag UnitType;

    /** Unit level */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    int32 UnitLevel = 1;
};

/**
 * Fragment for unit targeting information
 */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTargetFragment
{
    GENERATED_BODY()

    /** Target entity handle */
    FMassEntityHandle TargetEntity;

    /** Target location */
    FVector TargetLocation = FVector::ZeroVector;

    /** Target priority */
    float TargetPriority = 0.0f;

    /** Whether the unit has a valid target */
    bool HasTarget() const { return TargetEntity.IsValid() || !TargetLocation.IsZero(); }
};

/**
 * Fragment for unit ability information
 */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitAbilityFragment
{
    GENERATED_BODY()

    /** Ability handles for this unit */
    TArray<FGameplayAbilitySpecHandle> AbilityHandles;

    /** Active effect tags */
    TArray<FGameplayTag> ActiveEffectTags;

    /** Attribute values */
    TMap<FGameplayAttribute, float> AttributeValues;
};

/**
 * Fragment for unit team information
 */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTeamFragment
{
    GENERATED_BODY()

    /** Team ID */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    int32 TeamID = 0;

    /** Team color */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    FLinearColor TeamColor = FLinearColor::White;

    /** Team faction */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    FGameplayTag TeamFaction;
};

/**
 * Fragment for unit visual state
 */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVisualFragment
{
    GENERATED_BODY()

    /** Current animation tag */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    FGameplayTag CurrentAnimation;

    /** Target animation tag */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    FGameplayTag TargetAnimation;

    /** Blend alpha between animations */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    float BlendAlpha = 0.0f;

    /** Current LOD level */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    int32 LODLevel = 0;

    /** Whether the unit is visible */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    bool bIsVisible = true;

    /** Whether to use skeletal mesh instead of vertex animation */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    bool bUseSkeletalMesh = false;

    /** Index in the skeletal mesh pool if using skeletal mesh */
    UPROPERTY(EditAnywhere, Category = "Mass Unit")
    int32 SkeletalMeshIndex = -1;
};

/**
 * Fragment for unit formation information
 */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitFormationFragment
{
    GENERATED_BODY()

    /** Formation handle */
    int32 FormationHandle = -1;

    /** Formation slot */
    int32 FormationSlot = -1;

    /** Formation offset */
    FVector FormationOffset = FVector::ZeroVector;

    /** Whether the unit is in a formation */
    bool IsInFormation() const { return FormationHandle >= 0; }
};
