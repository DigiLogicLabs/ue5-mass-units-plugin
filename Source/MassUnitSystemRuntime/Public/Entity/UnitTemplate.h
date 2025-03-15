// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "MassEntityTypes.h"
#include "UnitTemplate.generated.h"

/**
 * Template for creating units in the Mass Unit System
 */
UCLASS(BlueprintType)
class MASSUNITSYSTEMRUNTIME_API UUnitTemplate : public UDataAsset
{
    GENERATED_BODY()

public:
    UUnitTemplate();

    /** Unit type tag */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template")
    FGameplayTag UnitType;

    /** Unit class tag */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template")
    FGameplayTag UnitClass;

    /** Base level for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template")
    int32 BaseLevel = 1;

    /** Base health for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template")
    int32 BaseHealth = 100;

    /** Base damage for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template")
    int32 BaseDamage = 10;

    /** Move speed for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template")
    float MoveSpeed = 300.0f;

    /** Default abilities for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template")
    TArray<FGameplayTag> DefaultAbilities;

    /** Skeletal mesh for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Visual")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    /** Static mesh for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Visual")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Vertex animation texture for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Visual")
    TSoftObjectPtr<UTexture2D> VertexAnimationTexture;

    /** Normal map texture for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Visual")
    TSoftObjectPtr<UTexture2D> NormalMapTexture;

    /** Animation tags for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Visual")
    TArray<FGameplayTag> AnimationTags;

    /** Default behavior tag for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Behavior")
    FGameplayTag DefaultBehavior;

    /** Default formation tag for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Behavior")
    FGameplayTag DefaultFormation;

    /** Team ID for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Team")
    int32 TeamID = 0;

    /** Team color for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Team")
    FLinearColor TeamColor = FLinearColor::White;

    /** Team faction tag for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Team")
    FGameplayTag TeamFaction;

    /** Base attributes for the unit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Attributes")
    TMap<FGameplayAttribute, float> BaseAttributes;

    /** Get the required fragments for this unit template */
    TArray<FMassFragmentRequirementDescription> GetRequiredFragments() const;
};
