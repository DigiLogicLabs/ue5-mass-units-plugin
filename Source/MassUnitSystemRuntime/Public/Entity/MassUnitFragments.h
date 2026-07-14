// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "Entity/MassEntityFallback.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "MassEntityTypes.h"
#include "MassUnitFragments.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UTexture2D;
class UAnimationAsset;

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

	/** Distance-LOD request; the bounded pool independently decides whether capacity is available. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	bool bWantsSkeletalMesh = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	float ViewerDistanceSquared = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> AnimationBlueprintClass;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> IdleAnimation = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> MoveAnimation = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> AttackAnimation = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> DeathAnimation = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> StunAnimation = nullptr;

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

	/** True only when PathPoints came from native navigation data rather than direct fallback. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Unit")
	bool bPathUsesNavmesh = false;

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
		bPathUsesNavmesh = false;
	}
};

USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitLODFragment : public FMassFragment
{
	GENERATED_BODY()
	int32 Level = 0;
	float NextUpdateTime = 0.0f;
};

USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitVisualizationLODFragment : public FMassFragment
{
	GENERATED_BODY()
	int32 LODLevel = 0;
};

/** Lightweight state used by the timer-driven crowd service and movement processor. */
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitCrowdFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector SteeringDirection = FVector::ZeroVector;
	FVector LastDestination = FVector::ZeroVector;
	FVector LastFollowTargetLocation = FVector::ZeroVector;
	FMassUnitEntityHandle InteractionPartner;
	float BaseMoveSpeed = 0.0f;
	float NextDecisionTime = 0.0f;
	float NextSteeringUpdateTime = 0.0f;
	float NextFollowUpdateTime = 0.0f;
	float InteractionEndTime = 0.0f;
	int32 GroupHandle = INDEX_NONE;
	int32 SubgroupIndex = 0;
	int32 SharedPathRevision = INDEX_NONE;
	int32 DecisionSequence = 0;
	int32 SimulationLOD = 0;
	float NavigationHeightOffset = 0.0f;
	bool bEnabled = false;
	bool bSleeping = false;
	bool bUse3DMovement = false;
	bool bConformToNavmeshHeight = false;

	void Reset()
	{
		SteeringDirection = FVector::ZeroVector;
		LastDestination = FVector::ZeroVector;
		LastFollowTargetLocation = FVector::ZeroVector;
		InteractionPartner.Invalidate();
		BaseMoveSpeed = 0.0f;
		NextDecisionTime = 0.0f;
		NextSteeringUpdateTime = 0.0f;
		NextFollowUpdateTime = 0.0f;
		InteractionEndTime = 0.0f;
		GroupHandle = INDEX_NONE;
		SubgroupIndex = 0;
		SharedPathRevision = INDEX_NONE;
		DecisionSequence = 0;
		SimulationLOD = 0;
		NavigationHeightOffset = 0.0f;
		bEnabled = false;
		bSleeping = false;
		bUse3DMovement = false;
		bConformToNavmeshHeight = false;
	}
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
