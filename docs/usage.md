# Usage

## Create and control units

Create a `UnitTemplate` Data Asset. Its values seed the native Mass fragments for every new unit: type/class tags, health, damage, movement speed, attack range/cooldown, team information, behavior/formation tags, and visual assets.

For a no-asset installation check, place `AMassUnitSpawner` in a level or call `Create Default Unit`. Both use the same native fragments and the engine-cube rendering fallback. The spawner is an ownership/bootstrap actor, not a representation actor; it stores handles and performs no per-frame work. See [quick-start.md](quick-start.md).

From Blueprint:

1. Call `Get Mass Unit Subsystem` with any object in the gameplay world.
2. Get `Unit Manager`.
3. Call `Create Unit From Template` and retain the returned `MassUnitHandle`.
4. Use the manager to read/write transforms, set a direct destination, assign or clear a target, apply lightweight damage, query units, or destroy them.

The placeable spawner also exposes `Spawn Units`, `Move Spawned Units By Offset`, `Command Spawned Units To Location`, `Destroy Spawned Units`, and valid-handle diagnostics for reusable encounter or wave Blueprints.

Handles contain a native Mass entity index and serial. Always call `Is Unit Valid` before reusing a long-lived handle; a destroyed entity's serial prevents an old handle from addressing a recycled entity index.

Movement, combat, and visibility processors register with the normal Mass processing phases. Do not tick them manually.

## Continuous crowds

For a placed spawner, enable `Enable Crowd Simulation`. The spawner remains non-ticking; it registers its native handles with the world-owned `UMassUnitCrowdSystem`, whose timer performs low-frequency decisions. The existing Mass movement processor still supplies smooth motion.

For project-owned waves, call `Register Crowd Group` with any valid handle array, center, and `MassUnitCrowdConfig`. Retain the returned integer handle and use:

- `Set Crowd Group Paused`
- `Set Crowd Group Center`
- `Force Crowd Group Update`
- `Unregister Crowd Group`
- `Get Crowd Stats`

Random choices are deterministic for a group seed, entity handle, and decision sequence. Separation uses nearby cells instead of all-pairs scanning. Social interactions set the lightweight `Interacting` state, face paired units toward each other, and broadcast `On Crowd Interaction Started`; they do not assign combat targets or require GAS.

Behavior LOD uses the nearest player-controller pawn/view as an observer. Farther units make decisions less often, and a group's `Max Simulation Distance` can sleep units beyond every observer. With no observer—such as a server-side test world—the service keeps the group active rather than incorrectly sleeping it.

## Navigation

`Set Unit Destination` writes a direct destination immediately. `Request Path` queues an asynchronous navmesh request and maps the result back to the exact entity that requested it. `Cancel Path` removes queued/in-flight work and clears the current path. The subsystem submits up to the configured request budget each tick.

If a world has no navigation data and direct fallback is enabled, the request still succeeds with the destination as a one-point path. This is useful for empty test maps and obstacle-free games.

## Formations

Use `Create Formation`, add unit handles, and call `Set Formation Target`. Supported shape names are:

- `Rectangle` (default)
- `Line`
- `Column`
- `Wedge`
- `Circle`

Formation membership writes each unit's formation fragment and destination. Destroying a formation removes its membership data; destroying a unit is also cleaned from formation registries on the next update.

## Rendering

The default rendering path groups visible units by static mesh into dynamic instanced static-mesh components with stable slots and batched transform updates. It chooses, in order:

1. The unit template's static mesh
2. The configured project fallback mesh
3. The engine cube

Units with skeletal meshes can use the bounded skeletal component pool at close range. This representation is intentionally capped.

When a custom Niagara system is configured, the plugin uploads these user parameters:

- `UnitCount` (int)
- `UnitPositions`, `UnitVelocities`, `UnitScales`, `UnitTeamColors` (vector arrays)
- `UnitRotations` (quaternion array)
- `UnitTeamIDs`, `UnitAnimationIndices`, `UnitAnimationTimes`, `UnitLODLevels`, `UnitVisibilityFlags` (float arrays)

The Niagara asset is optional. Configure it in **Project Settings > Plugins > Mass Unit System**.

## Gameplay Ability System

The bridge is opt-in and does not manufacture actors or Ability System Components for Mass entities.

1. Create/own an ASC using the project's normal GAS architecture.
2. Call `Register Ability System For Unit` with a valid unit handle and ASC.
3. Call `Grant Ability`, `Activate Ability`, or `Apply Gameplay Effect` through the bridge.
4. Unregister the ASC when the owning gameplay object is retired.

Lightweight Mass combat works without GAS.

For large battles, keep ambient movement, target acquisition, and coarse health in Mass. Register externally owned ASCs only for units currently requiring authoritative abilities/effects or player interaction, then demote/unregister them when that detail is no longer needed. The crowd interaction delegate is a convenient promotion hook but does not create ASCs automatically.

## Behavior Trees

Behavior Trees are also selective. `Set Behavior Tree` creates transient components only for registered units and synchronizes blackboard keys when they exist:

`Position`, `State`, `StateTime`, `UnitLevel`, `Health`, `TargetLocation`, `TargetPriority`, `HasTarget`, and `TeamID`.

Trees can command the entity with optional `RequestedTargetLocation` (Vector), `HasRequestedTarget` (Bool), `RequestedTargetPriority` (Float), `ClearTarget` (Bool), and `RequestedState` (Enum) keys. `ClearTarget` is consumed and reset. `Execute BT Task` writes the gameplay tag name to an optional Name key named `RequestedTask`; the project's tree decides how to consume it.

## C++ access

Include the specific public headers you use and add `MassUnitSystemRuntime` to the consuming module's dependencies.

```cpp
UMassUnitSubsystem* Units = UMassUnitSubsystem::Get(WorldContextObject);
UMassUnitEntityManager* Manager = Units ? Units->GetUnitManager() : nullptr;
FMassUnitHandle Handle = Manager
    ? Manager->CreateUnitFromTemplate(Template, SpawnTransform)
    : FMassUnitHandle{};
```

Avoid holding fragment pointers across Mass structural changes. Use the manager API for Blueprint-oriented code or create an `FMassEntityView` for short-lived native access.
