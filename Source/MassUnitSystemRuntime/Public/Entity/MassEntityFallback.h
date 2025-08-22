// Copyright Digi Logic Labs LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityFallback.generated.h"

/**
 * Custom fallback types for MassEntity plugin independence.
 * These allow the plugin to function even if UE MassEntity types are unavailable.
 */


USTRUCT(BlueprintType)
struct FMassUnitEntityHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 Index = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    int32 SerialNumber = 0;

    FMassUnitEntityHandle() = default;
    FMassUnitEntityHandle(int32 InIndex, int32 InSerialNumber) : Index(InIndex), SerialNumber(InSerialNumber) {}

    bool IsValid() const { return Index != INDEX_NONE; }
    void Invalidate() { Index = INDEX_NONE; }
    FString ToString() const { return FString::Printf(TEXT("Entity[%d:%d]"), Index, SerialNumber); }

    friend bool operator==(const FMassUnitEntityHandle& A, const FMassUnitEntityHandle& B)
    {
        return A.Index == B.Index && A.SerialNumber == B.SerialNumber;
    }
    friend bool operator!=(const FMassUnitEntityHandle& A, const FMassUnitEntityHandle& B)
    {
        return !(A == B);
    }
};

// Hash function for FMassUnitEntityHandle
FORCEINLINE uint32 GetTypeHash(const FMassUnitEntityHandle& Handle)
{
    return HashCombine(::GetTypeHash(Handle.Index), ::GetTypeHash(Handle.SerialNumber));
}


// Fallback entity view
USTRUCT(BlueprintType)
struct FMassUnitEntityView
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Unit")
    FMassUnitEntityHandle EntityHandle;

    FMassUnitEntityView() = default;
    FMassUnitEntityView(struct FMassUnitEntityManagerFallback& InEntityManager, FMassUnitEntityHandle InEntity)
        : EntityHandle(InEntity) {}

    template<typename FragmentType>
    bool HasFragmentData() const { return true; }
    template<typename FragmentType>
    FragmentType& GetFragmentData() { static FragmentType Fragment; return Fragment; }
    template<typename FragmentType>
    const FragmentType& GetFragmentData() const { static FragmentType Fragment; return Fragment; }
};

// Fallback entity manager
USTRUCT(BlueprintType)
struct FMassUnitEntityManagerFallback
{
    GENERATED_BODY()

    bool IsEntityValid(const FMassUnitEntityHandle& Entity) const { return Entity.IsValid(); }

    // Stub: Create a new entity handle
    FMassUnitEntityHandle CreateEntity() { static int32 NextIndex = 0; return FMassUnitEntityHandle(++NextIndex, 1); }

    // Stub: Destroy an entity (no-op)
    void DestroyEntity(const FMassUnitEntityHandle& Entity) { /* no-op for fallback */ }
};

// Fallback entity subsystem
UCLASS()
class MASSUNITSYSTEMRUNTIME_API UMassUnitEntitySubsystem : public UObject
{
    GENERATED_BODY()
public:
    UMassUnitEntitySubsystem() {}
    virtual ~UMassUnitEntitySubsystem() {}

    FMassUnitEntityManagerFallback* GetMutableUnitEntityManager() { static FMassUnitEntityManagerFallback Manager; return &Manager; }
};
UINTERFACE(BlueprintType)
class MASSUNITSYSTEMRUNTIME_API UMassUnitEntitySubsystemInterface : public UInterface
{
    GENERATED_BODY()
};

class MASSUNITSYSTEMRUNTIME_API IMassUnitEntitySubsystemInterface
{
    GENERATED_BODY()
public:
    // Add interface methods here
};

// Fallback entity query
struct FMassUnitEntityQuery {
    template<typename FragmentType>
    void AddRequirement(int /*access*/) {}

    template<typename TagType>
    void AddTagRequirement(int /*presence*/) {}

    template<typename EntityManagerType, typename ExecutionContextType, typename FuncType>
    void ForEachEntityChunk(EntityManagerType& EntityManager, ExecutionContextType& Context, FuncType&& Func) {
        // Fallback: just call Func with Context
        Func(Context);
    }
};

// Custom fragment requirement descriptor
struct FMassUnitFragmentRequirementDescription {
    UScriptStruct* FragmentType;
    FMassUnitFragmentRequirementDescription(UScriptStruct* InType) : FragmentType(InType) {}
};

struct FMassUnitFragmentRequirements {};
struct FMassUnitCommandBuffer {};
struct FMassUnitExecutionContext {
    int32 NumEntities = 1;

    template<typename FragmentType>
    TArrayView<FragmentType> GetMutableFragmentView() {
        static TArray<FragmentType> DummyList;
        DummyList.SetNum(NumEntities);
        return TArrayView<FragmentType>(DummyList.GetData(), DummyList.Num());
    }
    template<typename FragmentType>
    TConstArrayView<FragmentType> GetFragmentView() const {
        static TArray<FragmentType> DummyList;
        DummyList.SetNum(NumEntities);
        return TConstArrayView<FragmentType>(DummyList.GetData(), DummyList.Num());
    }
    int32 GetNumEntities() const { return NumEntities; }
    FMassUnitEntityHandle GetEntity(int32 Index) const { return FMassUnitEntityHandle(Index, 1); }
    float GetDeltaTimeSeconds() const { return 1.0f; }
};
enum class EMassUnitExecutionFlags {};
