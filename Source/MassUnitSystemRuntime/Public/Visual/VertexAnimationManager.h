// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "VertexAnimationManager.generated.h"

/**
 * Manager for vertex animation textures and transitions in the Mass Unit System
 */
UCLASS(BlueprintType, Blueprintable)
class MASSUNITSYSTEMRUNTIME_API UVertexAnimationManager : public UObject
{
    GENERATED_BODY()

public:
    UVertexAnimationManager();
    virtual ~UVertexAnimationManager();

    /** Initialize the vertex animation manager */
    void Initialize();
    
    /** Deinitialize the vertex animation manager */
    void Deinitialize();
    
    /** Get animation texture for a specific animation tag */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UTexture2D* GetAnimationTexture(FGameplayTag AnimationTag);
    
    /** Blend between two animations */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    UTexture2D* BlendAnimations(FGameplayTag FromAnim, FGameplayTag ToAnim, float BlendAlpha);
    
    /** Get animation index for a specific animation tag */
    UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
    int32 GetAnimationIndex(FGameplayTag AnimationTag);

private:
    /** Map of animation tags to textures */
    UPROPERTY(Transient)
    TMap<FGameplayTag, UTexture2D*> AnimationTextureMap;
    
    /** Map of animation tags to indices */
    UPROPERTY(Transient)
    TMap<FGameplayTag, int32> AnimationIndexMap;
    
    /** Map of blended animation pairs to textures */
    UPROPERTY(Transient)
    TMap<FString, UTexture2D*> BlendedAnimationMap;
    
    /** Load animation textures */
    void LoadAnimationTextures();
    
    /** Create a blended animation texture */
    UTexture2D* CreateBlendedTexture(UTexture2D* FromTexture, UTexture2D* ToTexture, float BlendAlpha);
    
    /** Get blend key for two animations */
    FString GetBlendKey(FGameplayTag FromAnim, FGameplayTag ToAnim, float BlendAlpha);
};
