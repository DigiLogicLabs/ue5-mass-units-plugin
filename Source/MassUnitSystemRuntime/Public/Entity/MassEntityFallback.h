// Copyright Your Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityFallback.generated.h"

/**
 * Custom implementation for entity types without relying on MassEntity.
 * This allows the plugin to function as a standalone system.
 */

// Basic entity handle
struct FMassEntityHandle
{
    int32 Index = INDEX_NONE;
    int32 SerialNumber = 0;

    FMassEntityHandle() = default;
    FMassEntityHandle(int32 InIndex, int32 InSerialNumber) : Index(InIndex), SerialNumber(InSerialNumber) {}

    bool IsValid() const { return Index != INDEX_NONE; }
    void Invalidate() { Index = INDEX_NONE; }
    FString ToString() const { return FString::Printf(TEXT("Entity[%d:%d]"), Index, SerialNumber); }

    friend bool operator==(const FMassEntityHandle& A, const FMassEntityHandle& B)
    {
        return A.Index == B.Index && A.SerialNumber == B.SerialNumber;
    }

    friend bool operator!=(const FMassEntityHandle& A, const FMassEntityHandle& B)
    {
        return !(A == B);
    }
};

// Basic transform fragment
USTRUCT()
struct MASSUNITSYSTEMRUNTIME_API FMassUnitTransformFragment
{
    GENERATED_BODY()

    FTransform Transform;

    FTransform GetTransform() const { return Transform; }
    void SetTransform(const FTransform& InTransform) { Transform = InTransform; }
};

// Define FTransformFragment as an alias to our custom one
typedef FMassUnitTransformFragment FTransformFragment;

// Basic entity view
class MASSUNITSYSTEMRUNTIME_API FMassEntityView
{
public:
    FMassEntityView() = default;
    
    FMassEntityView(class FMassEntityManager& EntityManager, FMassEntityHandle Entity)
        : EntityHandle(Entity)
    {
    }

    template<typename FragmentType>
    bool HasFragmentData() const
    {
        // In our implementation, we'll assume all fragments are available
        return true;
    }

    template<typename FragmentType>
    FragmentType& GetFragmentData()
    {
        // Return a default-constructed fragment
        static FragmentType Fragment;
        return Fragment;
    }

    template<typename FragmentType>
    const FragmentType& GetFragmentData() const
    {
        // Return a default-constructed fragment
        static FragmentType Fragment;
        return Fragment;
    }

private:
    FMassEntityHandle EntityHandle;
};

// Basic entity manager
class MASSUNITSYSTEMRUNTIME_API FMassEntityManager
{
public:
    bool IsEntityValid(const FMassEntityHandle& Entity) const
    {
        return Entity.IsValid();
    }
};

// Basic entity subsystem interface
UINTERFACE()
class MASSUNITSYSTEMRUNTIME_API UMassEntitySubsystemInterface : public UInterface
{
    GENERATED_BODY()
};

class MASSUNITSYSTEMRUNTIME_API IMassEntitySubsystemInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mass Entity")
    class UMassEntitySubsystem* GetMassEntitySubsystem();
};

// Basic entity subsystem
UCLASS()
class MASSUNITSYSTEMRUNTIME_API UMassUnitEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    FMassEntityManager& GetMutableEntityManager()
    {
        static FMassEntityManager EntityManager;
        return EntityManager;
    }
};

// Define UMassEntitySubsystem as an alias to our custom subsystem
typedef UMassUnitEntitySubsystem UMassEntitySubsystem;

// Additional types needed for the plugin
struct FMassExecutionContext {};
struct FMassEntityQuery {};
enum class EMassExecutionFlags {};
struct FMassFragmentRequirements {};
struct FMassCommandBuffer {};
