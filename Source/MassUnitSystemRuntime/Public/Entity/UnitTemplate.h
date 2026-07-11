// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UnitTemplate.generated.h"

class UScriptStruct;
class USkeletalMesh;
class UStaticMesh;
class UTexture2D;

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

    /** Maximum distance at which this unit can perform its basic attack. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Combat", meta = (ClampMin = "0.0", ForceUnits = "cm"))
    float AttackRange = 200.0f;

    /** Time between basic attacks. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit Template|Combat", meta = (ClampMin = "0.0", ForceUnits = "s"))
    float AttackCooldown = 1.0f;

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

    /** Native Mass fragments used by every unit archetype created from this template. */
    TArray<const UScriptStruct*> GetRequiredFragments() const;
};
