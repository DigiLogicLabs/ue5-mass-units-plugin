// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity/MassEntityFallback.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "MassEntityTypes.h"
#include "MassUnitFragments.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UTexture2D;

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

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTransformFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FTransform Transform;

	const FTransform& GetTransform() const { return Transform; }
	FTransform& GetMutableTransform() { return Transform; }
	void SetTransform(const FTransform& InTransform) { Transform = InTransform; }
};

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitStateFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	EMassUnitState CurrentState = EMassUnitState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit", meta = (ClampMin = "0.0"))
	float StateTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FGameplayTag UnitType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FGameplayTag UnitClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FGameplayTag DefaultBehavior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit", meta = (ClampMin = "1"))
	int32 UnitLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit|Combat", meta = (ClampMin = "0.0"))
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit|Combat", meta = (ClampMin = "0.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit|Combat", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit|Combat", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit|Combat", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float AttackCooldown = 1.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Mass Unit|Combat")
	float AttackCooldownRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit|Movement", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	float MoveSpeed = 300.0f;
};

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTargetFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FMassUnitEntityHandle TargetEntity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FVector TargetLocation = FVector::ZeroVector;

	/** Explicit flag so the world origin remains a valid destination. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	bool bHasTargetLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	float TargetPriority = 0.0f;

	bool HasTarget() const { return TargetEntity.IsValid() || bHasTargetLocation; }
	void Clear() { TargetEntity.Invalidate(); TargetLocation = FVector::ZeroVector; bHasTargetLocation = false; TargetPriority = 0.0f; }
};

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitAbilityFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Mass Unit")
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Mass Unit")
	TArray<FGameplayTag> ActiveEffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	TArray<FGameplayTag> DefaultAbilityTags;
};

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTeamFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	int32 TeamID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FLinearColor TeamColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FGameplayTag TeamFaction;
};

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVisualFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FGameplayTag CurrentAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FGameplayTag TargetAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BlendAlpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	int32 LODLevel = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	bool bIsVisible = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	bool bUseSkeletalMesh = false;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> VertexAnimationTexture = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> NormalMapTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	TArray<FGameplayTag> AnimationTags;
};

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitFormationFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	int32 FormationHandle = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	int32 FormationSlot = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	FVector FormationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
	FGameplayTag DefaultFormation;

	bool IsInFormation() const { return FormationHandle != INDEX_NONE; }
};

USTRUCT(BlueprintType)
struct MASSUNITSYSTEMRUNTIME_API FMassUnitNavigationFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	FVector DestinationLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	TArray<FVector> PathPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	int32 CurrentPathIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	bool bPathRequested = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	bool bPathValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit", meta = (ClampMin = "1.0", ForceUnits = "cm"))
	float AcceptanceRadius = 50.0f;

	bool HasReachedDestination() const
	{
		return bPathValid && PathPoints.IsValidIndex(CurrentPathIndex) == false;
	}

	void ResetPath()
	{
		PathPoints.Reset();
		CurrentPathIndex = INDEX_NONE;
		bPathRequested = false;
		bPathValid = false;
	}
};

USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitLODFragment : public FMassFragment
{
	GENERATED_BODY()
	int32 Level = 0;
};

USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVisualizationLODFragment : public FMassFragment
{
	GENERATED_BODY()
	int32 LODLevel = 0;
};

// These fragments intentionally own dynamic/reflected data. Mass supports such
// fragments, but requires authors to acknowledge their non-trivial copy cost.
template<>
struct TMassFragmentTraits<FMassUnitAbilityFragment>
{
	enum { AuthorAcceptsItsNotTriviallyCopyable = true };
};

template<>
struct TMassFragmentTraits<FMassUnitVisualFragment>
{
	enum { AuthorAcceptsItsNotTriviallyCopyable = true };
};

template<>
struct TMassFragmentTraits<FMassUnitNavigationFragment>
{
	enum { AuthorAcceptsItsNotTriviallyCopyable = true };
};
