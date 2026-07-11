# Public API

## Entry point

`UMassUnitSubsystem` is a `UTickableWorldSubsystem` available in Game, PIE, and Game Preview worlds. Resolve it with `UMassUnitSubsystem::Get(WorldContextObject)` or Unreal's world-subsystem API.

It owns and exposes:

- `UMassUnitEntityManager`
- `UFormationSystem`
- `UMassUnitNavigationSystem`
- `UNiagaraUnitSystem`
- `UUnitMeshPool`
- `UGASUnitIntegration`
- `UMassUnitBehaviorIntegration`
- `UUnitGameplayEventSystem`

## Handles and templates

`FMassUnitHandle` is the Blueprint-facing wrapper. Its `EntityHandle` is an `FMassUnitEntityHandle`, which preserves the index and serial of Unreal's native `FMassEntityHandle` and converts back for native APIs.

`UUnitTemplate` is the creation Data Asset. `GetRequiredFragments()` describes the native archetype used by the manager.

## Unit manager

Primary Blueprint functions:

- `CreateUnitFromTemplate`
- `DestroyUnit`, `IsUnitValid`, `GetUnitCount`, `GetAllUnits`
- `GetUnitsByType`, `GetUnitsByTeam`
- `GetUnitTransform`, `SetUnitTransform`
- `SetUnitDestination`
- `SetUnitTarget`, `ClearUnitTarget`
- `GetUnitState`, `ApplyDamage`

The `Internal` variants accept native-compatible `FMassUnitEntityHandle` values for C++ systems.

## Navigation and formations

`UMassUnitNavigationSystem::RequestPath` queues a request. `ProcessPathRequests` exists for explicit use but is already called by the world subsystem.

`UFormationSystem` creates integer formation handles and supports add/remove, target, shape, location/rotation, and member queries.

## Optional bridges

`UGASUnitIntegration` maps valid Mass handles to externally owned ASCs and exposes ability/effect operations.

`UMassUnitBehaviorIntegration` attaches or removes transient Behavior Tree/Blackboard components and synchronizes recognized keys.

`UUnitGameplayEventSystem` registers gameplay-tag callbacks and dispatches payloads without coupling every unit to a UObject actor.

## Native fragments and processors

`MassUnitFragments.h` declares the plugin's transform, state, target, ability, team, visual, formation, navigation, and LOD fragments. `MassUnitCommonFragments.h` provides velocity, force, and look-direction fragments. Non-trivial fragments explicitly opt into Mass fragment traits.

The runtime module provides auto-registered movement, combat, and visibility processors. `UMassUnitFormationProcessor` remains a disabled compatibility processor; formation work is owned by `UFormationSystem` to avoid double updates.
