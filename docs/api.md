# Public API

## Entry point

`UMassUnitSubsystem` is a `UTickableWorldSubsystem` available in Game, PIE, and Game Preview worlds. Resolve it with `UMassUnitSubsystem::Get(WorldContextObject)` or Unreal's world-subsystem API.

It owns and exposes:

- `UMassUnitEntityManager`
- `UFormationSystem`
- `UMassUnitNavigationSystem`
- `UMassUnitCrowdSystem`
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

- `CreateDefaultUnit` for an asset-free visible baseline
- `CreateUnitFromTemplate`
- `DestroyUnit`, `IsUnitValid`, `GetUnitCount`, `GetAllUnits`
- `GetUnitsByType`, `GetUnitsByTeam`
- `GetUnitTransform`, `SetUnitTransform`
- `SetUnitDestination`
- `SetUnitTarget`, `ClearUnitTarget`
- `GetUnitState`, `ApplyDamage`

The `Internal` variants accept native-compatible `FMassUnitEntityHandle` values for C++ systems.

## Quick-start spawner

`AMassUnitSpawner` is a placeable, non-ticking ownership actor. Its setup fields are Blueprint-readable/writable. It can create a centered grid from a `UnitTemplate` or the runtime default, issue direct/navmesh commands while preserving spacing, opt into continuous crowd behavior, expose valid owned handles, and destroy only its own units on end play. `Spawn On Authority Only` is enabled by default to avoid accidental client/server duplication.

The spawner is optional. Production wave managers can call the same unit-manager, navigation, and formation APIs directly.

## Navigation and formations

`UMassUnitNavigationSystem::RequestPath` queues a request. `CancelPath` cancels queued/in-flight work for one handle. `ProcessPathRequests` exists for explicit use but is already called by the world subsystem.

`UFormationSystem` creates integer formation handles and supports add/remove, target, shape, location/rotation, and member queries.

`UNiagaraUnitSystem` exposes `IsUsingNiagara`, `GetInstancedMeshComponentCount`, `GetInstancedMeshInstanceCount`, and `GetInstancedMeshTopologyRevision` for first-run rendering and topology-churn diagnostics.

## Crowd behavior

`UMassUnitCrowdSystem` registers arbitrary handle arrays as integer crowd groups. `FMassUnitCrowdConfig` exposes bounded wander, speed/idle randomness, spatial separation, paired interactions, deterministic seed, simulation distance, and visual debugging. Group APIs support register/unregister, pause/resume, center changes, forced decisions, counts, and `FMassUnitCrowdStats`. `OnCrowdInteractionStarted` is a Blueprint multicast hook for project-specific reactions.

## Optional bridges

`UGASUnitIntegration` maps valid Mass handles to externally owned ASCs and exposes ability/effect operations.

`UMassUnitBehaviorIntegration` attaches or removes transient Behavior Tree/Blackboard components and synchronizes recognized keys.

`UUnitGameplayEventSystem` registers gameplay-tag callbacks and dispatches payloads without coupling every unit to a UObject actor.

## Native fragments and processors

`MassUnitFragments.h` declares the plugin's transform, state, target, ability, team, visual, formation, navigation, crowd, and LOD fragments. `MassUnitCommonFragments.h` provides velocity, force, and look-direction fragments. Non-trivial fragments explicitly opt into Mass fragment traits.

The runtime module provides auto-registered movement, combat, and visibility processors. `UMassUnitFormationProcessor` remains a disabled compatibility processor; formation work is owned by `UFormationSystem` to avoid double updates.
