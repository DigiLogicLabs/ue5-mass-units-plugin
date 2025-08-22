// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Visual/VertexAnimationManager.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "RenderingThread.h"
#include "TextureResource.h"

UVertexAnimationManager::UVertexAnimationManager()
{
}

UVertexAnimationManager::~UVertexAnimationManager()
{
}

void UVertexAnimationManager::Initialize()
{
    // Load animation textures
    LoadAnimationTextures();
    
    UE_LOG(LogTemp, Log, TEXT("VertexAnimationManager: Initialized with %d animations"), AnimationTextureMap.Num());
}

void UVertexAnimationManager::Deinitialize()
{
    // Clear animation maps
    AnimationTextureMap.Empty();
    AnimationIndexMap.Empty();
    BlendedAnimationMap.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VertexAnimationManager: Deinitialized"));
}

UTexture2D* UVertexAnimationManager::GetAnimationTexture(FGameplayTag AnimationTag)
{
    // Check if animation exists
    if (UTexture2D** TexturePtr = AnimationTextureMap.Find(AnimationTag))
    {
        return *TexturePtr;
    }
    
    // Return default texture if not found
    UE_LOG(LogTemp, Warning, TEXT("VertexAnimationManager: Animation texture not found for tag %s"), *AnimationTag.ToString());
    return nullptr;
}

UTexture2D* UVertexAnimationManager::BlendAnimations(FGameplayTag FromAnim, FGameplayTag ToAnim, float BlendAlpha)
{
    // Clamp blend alpha
    BlendAlpha = FMath::Clamp(BlendAlpha, 0.0f, 1.0f);
    
    // If blend alpha is 0, return from animation
    if (BlendAlpha <= 0.0f)
    {
        return GetAnimationTexture(FromAnim);
    }
    
    // If blend alpha is 1, return to animation
    if (BlendAlpha >= 1.0f)
    {
        return GetAnimationTexture(ToAnim);
    }
    
    // Get blend key
    FString BlendKey = GetBlendKey(FromAnim, ToAnim, BlendAlpha);
    
    // Check if blended animation exists
    if (UTexture2D** BlendedTexturePtr = BlendedAnimationMap.Find(BlendKey))
    {
        return *BlendedTexturePtr;
    }
    
    // Get source textures
    UTexture2D* FromTexture = GetAnimationTexture(FromAnim);
    UTexture2D* ToTexture = GetAnimationTexture(ToAnim);
    
    // Skip if either texture is missing
    if (!FromTexture || !ToTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("VertexAnimationManager: Cannot blend animations - missing textures"));
        return FromTexture ? FromTexture : ToTexture;
    }
    
    // Create blended texture
    UTexture2D* BlendedTexture = CreateBlendedTexture(FromTexture, ToTexture, BlendAlpha);
    
    // Add to map
    if (BlendedTexture)
    {
        BlendedAnimationMap.Add(BlendKey, BlendedTexture);
    }
    
    return BlendedTexture;
}

int32 UVertexAnimationManager::GetAnimationIndex(FGameplayTag AnimationTag)
{
    // Check if animation index exists
    if (int32* IndexPtr = AnimationIndexMap.Find(AnimationTag))
    {
        return *IndexPtr;
    }
    
    // Return -1 if not found
    UE_LOG(LogTemp, Warning, TEXT("VertexAnimationManager: Animation index not found for tag %s"), *AnimationTag.ToString());
    return -1;
}

void UVertexAnimationManager::LoadAnimationTextures()
{
    // In a real implementation, this would load animation textures from a path or asset registry
    // For this example, we'll create some dummy animation tags and indices
    
    // Define common animation tags
    TArray<FGameplayTag> AnimationTags;
    AnimationTags.Add(FGameplayTag::RequestGameplayTag(FName("Animation.Idle")));
    AnimationTags.Add(FGameplayTag::RequestGameplayTag(FName("Animation.Walk")));
    AnimationTags.Add(FGameplayTag::RequestGameplayTag(FName("Animation.Run")));
    AnimationTags.Add(FGameplayTag::RequestGameplayTag(FName("Animation.Attack")));
    AnimationTags.Add(FGameplayTag::RequestGameplayTag(FName("Animation.Death")));
    AnimationTags.Add(FGameplayTag::RequestGameplayTag(FName("Animation.Stun")));
    
    // Assign indices
    for (int32 i = 0; i < AnimationTags.Num(); ++i)
    {
        AnimationIndexMap.Add(AnimationTags[i], i);
    }
    
    // In a real implementation, we would load actual textures here
    // For this example, we'll just log that we're loading them
    UE_LOG(LogTemp, Log, TEXT("VertexAnimationManager: Loading animation textures (simulated)"));
    
    // For a real implementation, we would do something like:
    /*
    for (const FGameplayTag& AnimTag : AnimationTags)
    {
        FString TexturePath = FString::Printf(TEXT("/Game/MassUnitSystem/Textures/VAT_%s"), *AnimTag.ToString().Replace(TEXT("."), TEXT("_")));
        UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *TexturePath);
        if (Texture)
        {
            AnimationTextureMap.Add(AnimTag, Texture);
        }
    }
    */
}

UTexture2D* UVertexAnimationManager::CreateBlendedTexture(UTexture2D* FromTexture, UTexture2D* ToTexture, float BlendAlpha)
{
    // In a real implementation, this would create a blended texture by interpolating between two textures
    // For this example, we'll just log that we're creating it
    UE_LOG(LogTemp, Log, TEXT("VertexAnimationManager: Creating blended texture (simulated) with alpha %.2f"), BlendAlpha);
    
    // For a real implementation, we would do something like:
    /*
    // Create new texture
    UTexture2D* BlendedTexture = UTexture2D::CreateTransient(
        FromTexture->GetSizeX(),
        FromTexture->GetSizeY(),
        FromTexture->GetPixelFormat()
    );
    
    // Lock textures for reading/writing
    uint8* FromData = nullptr;
    uint8* ToData = nullptr;
    uint8* BlendedData = nullptr;
    
    // Get texture data
    FromTexture->Source.GetMipData(FromData, 0);
    ToTexture->Source.GetMipData(ToData, 0);
    BlendedTexture->Source.GetMipData(BlendedData, 0);
    
    // Blend textures
    int32 PixelCount = FromTexture->GetSizeX() * FromTexture->GetSizeY();
    for (int32 i = 0; i < PixelCount; ++i)
    {
        // For each pixel component (assuming RGBA)
        for (int32 j = 0; j < 4; ++j)
        {
            int32 Index = i * 4 + j;
            BlendedData[Index] = FMath::Lerp(FromData[Index], ToData[Index], BlendAlpha);
        }
    }
    
    // Update texture
    BlendedTexture->UpdateResource();
    
    return BlendedTexture;
    */
    
    // For this example, just return the FromTexture
    return FromTexture;
}

FString UVertexAnimationManager::GetBlendKey(FGameplayTag FromAnim, FGameplayTag ToAnim, float BlendAlpha)
{
    // Create a unique key for this blend
    return FString::Printf(TEXT("%s_%s_%.2f"), *FromAnim.ToString(), *ToAnim.ToString(), BlendAlpha);
}
