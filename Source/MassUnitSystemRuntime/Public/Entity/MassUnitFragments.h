
// ...existing code...

// --- Custom LOD Fragments ---

// ...existing code...
// ...existing code...

#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "Entity/MassEntityFallback.h"
#include "MassUnitFragments.generated.h"

// --- Custom LOD Fragments ---

/**
 * Fragment for unit LOD state
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitLODFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 Level = 0;
};

/**
 * Fragment for unit visualization LOD state
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVisualizationLODFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 LODLevel = 0;
};

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTransformFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FTransform Transform;

    FTransform GetTransform() const { return Transform; }
    void SetTransform(const FTransform& InTransform) { Transform = InTransform; }
};

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
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitStateFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    EMassUnitState CurrentState = EMassUnitState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    float StateTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FGameplayTag UnitType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 UnitLevel = 1;
};

/**
 * Fragment for unit targeting information
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTargetFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FMassUnitEntityHandle TargetEntity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    float TargetPriority = 0.0f;

    bool HasTarget() const { return TargetEntity.IsValid() || !TargetLocation.IsZero(); }
};

/**
 * Fragment for unit ability information
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitAbilityFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    TArray<FGameplayAbilitySpecHandle> AbilityHandles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    TArray<FGameplayTag> ActiveEffectTags;

    // ...existing code...
};

/**
 * Fragment for unit team information
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTeamFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 TeamID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FLinearColor TeamColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FGameplayTag TeamFaction;
};

/**
 * Fragment for unit visual state
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVisualFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FGameplayTag CurrentAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FGameplayTag TargetAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    float BlendAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 LODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    bool bIsVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    bool bUseSkeletalMesh = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 SkeletalMeshIndex = -1;
};

/**
 * Fragment for unit formation information
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitFormationFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 FormationHandle = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 FormationSlot = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FVector FormationOffset = FVector::ZeroVector;

    bool IsInFormation() const { return FormationHandle >= 0; }
};

/**
 * Fragment for unit navigation information
 */
USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitNavigationFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FVector DestinationLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 CurrentPathIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    bool bPathRequested = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    bool bPathValid = false;

    bool HasReachedDestination() const 
    { 
        return PathPoints.Num() == 0 || 
              (CurrentPathIndex >= PathPoints.Num() - 1 && 
               FVector::DistSquared(DestinationLocation, PathPoints.Last()) < 100.0f); 
    }
};
// Copyright Digi Logic Labs LLC. All Rights Reserved.
#pragma once
