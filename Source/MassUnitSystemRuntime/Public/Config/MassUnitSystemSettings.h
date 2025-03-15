// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MassUnitSystemSettings.generated.h"

class UUnitConfigDataAsset;

/**
 * Global settings for the Mass Unit System
 */
UCLASS(BlueprintType, config=Game, defaultconfig)
class MASSUNITSYSTEMRUNTIME_API UMassUnitSystemSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    UMassUnitSystemSettings();

    /** Maximum number of units to simulate */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Performance")
    int32 MaxUnits;
    
    /** Maximum number of units to render with skeletal meshes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Performance")
    int32 MaxSkeletalMeshUnits;
    
    /** Maximum number of path requests per frame */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Performance")
    int32 MaxPathRequestsPerFrame;
    
    /** Enable debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Debug")
    bool bEnableDebugVisualization;
    
    /** Debug visualization duration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Debug")
    float DebugVisualizationDuration;
    
    /** Available unit configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Units")
    TArray<TSoftObjectPtr<UUnitConfigDataAsset>> UnitConfigurations;
    
    /** Default unit configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Units")
    TSoftObjectPtr<UUnitConfigDataAsset> DefaultUnitConfiguration;
    
    /** Get the settings object */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    static UMassUnitSystemSettings* Get();
};
