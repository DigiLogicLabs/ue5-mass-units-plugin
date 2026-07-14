# Changelog

## 1.4.0

- Added opt-in group-level player engagement with manual, interaction/damage, proximity, and always-acquire modes.
- Added budget-compatible moving-target sampling, repath thresholds, deterministic follow spread, automatic release/return-to-wander, and Blueprint lifecycle/attack events.
- Added one shared navmesh corridor per engaged group, waypoint look-ahead, spatial-hash separation while following, group-only debug by default, and diagnostics proving that the default path issues zero per-unit nav queries.
- Added deterministic managed subgroups with shared ambient corridors, bounded shared-path work, subgroup membership queries, and coalesced presentation cues for pooled sound/VFX.
- Added terrain-height conformance for Planar 2D navmesh paths, including a configurable mesh-pivot offset and no per-unit ground trace.
- Added planar and true free-3D crowd modes, including 3D spatial hashing, sphere-volume wandering, XYZ separation, and direct pitched movement.
- Added native health queries, healing, health/death events, nearest-unit and radius queries, plus a one-call damage-and-activate path.
- Added standard Actor damage and optional GAS Gameplay Effects for engaged Actor targets without allocating an ASC or Actor per Mass entity.
- Added skeletal representation hysteresis/capacity diagnostics, state clip and Animation Blueprint inputs, stable tag-based VAT indices, and eight-float ISM animation/LOD/team/health data.
- Extended the placeable spawner, diagnostics, README workflow, and packaged automation coverage for engagement, subgroup navigation, terrain height, representation data, and free-3D behavior.

## 1.3.0

- Added a world-owned, timer-driven crowd service with deterministic bounded wandering, random speed/idle variation, pause/resume, movable centers, forced decisions, and Blueprint diagnostics.
- Added spatial-hash separation steering and paired non-combat interaction events without per-unit Actors, UObjects, collision components, or Behavior Trees.
- Added behavior LOD distance multipliers, round-robin decision budgets, observer-based simulation sleeping, and staggered multi-view visual LOD refresh intervals.
- Added safe queued/in-flight navigation cancellation for crowd pause, interaction, sleeping, and unregister paths.
- Added spawner opt-in controls plus automation coverage for crowd destinations, deterministic variation, interactions, pause/resume, and cleanup.

## 1.2.1

- Replaced the moving-unit HISM fallback with dynamic ISM components so continuous movement does not rebuild hierarchical cluster trees.
- Replaced per-refresh clear/hide/re-add rendering with stable slots, batched in-place transform updates, and add/remove deltas only when topology changes.
- Added a Blueprint topology-revision diagnostic and regression coverage for flicker-free transform updates and cleanup.

## 1.2.0

- Added a placeable, non-ticking Mass Unit Spawner with Blueprint-writable scale, layout, movement, lifecycle, networking, and debug controls for a zero-asset blank-level workflow.
- Added asset-free `Create Default Unit` and explicit `Get Mass Unit Subsystem` Blueprint discovery metadata.
- Added instanced-mesh diagnostics and automation coverage for default creation, spawner ownership, movement commands, rendering fallback, and cleanup.
- Added a five-minute quick-start guide and repeatable production setup pattern.

## 1.1.0

- Replaced the shared static fallback entity store with native world-owned Mass entities and fragments.
- Added stable Blueprint-compatible wrappers around native entity handles.
- Implemented functional movement, combat, visibility, navigation, formation, and destruction paths.
- Changed plugin bootstrap to a tickable world subsystem with a native Mass dependency.
- Added an asset-free HISM renderer and made custom Niagara rendering optional/configurable.
- Implemented bounded skeletal mesh pooling and truthful vertex-animation tag/index registration.
- Reworked GAS as an opt-in bridge to externally owned ASCs; removed GASCompanion references/dependency.
- Implemented an optional selective Behavior Tree/Blackboard bridge.
- Removed public headers that shadowed Unreal Engine Mass headers.
- Reduced module and plugin dependencies to the features actually used.
- Added a native lifecycle automation test, safe world shutdown, portable build helpers, and current setup/API documentation.
- Validated UAT packaging with UE 5.7.2 for Win64 Editor, Development, and Shipping targets.

## 1.0.0

- Initial repository version.
