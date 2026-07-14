# Mass Unit System: Current Status and Validation Backlog

Last updated: 2026-07-14

This is the active implementation and release checklist. The immediate goal is a repeatable blank-project install that scales from visible test cubes to project-owned static/VAT/skeletal units and interoperates with GAS without creating an Actor, collision component, audio component, or Ability System Component for every Mass entity.

## Current working baseline

- [x] Plugin lives entirely under `Plugins/ue5-mass-units-plugin`; no required host-project source changes.
- [x] Asset-free `Mass Unit Spawner` creates visible engine-cube Mass entities in `/Game/Levels/Testing.Testing`.
- [x] Cubes move continuously without the earlier representation blinking/topology rebuild.
- [x] Planar and free-3D crowd modes, deterministic wander, separation, interactions, behavior LOD, simulation sleeping, and update budgets are implemented.
- [x] Interaction/manual/proximity/always player engagement, dynamic following, attack requests, native Mass health, Actor damage, and optional target Gameplay Effects are implemented.
- [x] The live cube example has previously acquired the player, followed a moved player, entered attack range, and produced cooldown attack requests.
- [x] Group engagement uses one target sample and a shared corridor by default; red per-unit arrows are optional debug destinations, not raycasts.
- [x] Source now partitions large crowds into deterministic managed subgroups and uses one ambient navigation corridor per subgroup instead of one wander path per entity.
- [x] Source now carries real navmesh elevation into Planar2D movement with a configurable mesh-pivot height offset and no per-unit ground trace.
- [x] Live transient PIE validation moved 25 cubes from flat `Z = 150` onto a hill at `Z = 239-266`; all units remained within 5.74 cm of `terrain Z + 50 cm` during ambient travel.
- [x] Live engaged validation used one shared corridor, zero individual path requests, terrain-projected final spread slots, and produced cooldown attack requests.
- [x] Source now exposes coalesced subgroup presentation cues for pooled MetaSound/audio/Niagara reactions.
- [x] Source now provides skeletal representation hysteresis, capacity diagnostics, state-clip/Animation Blueprint inputs, correct tag-based VAT indices, and eight ISM per-instance material values.

## Validation currently in progress

- [x] Complete the clean UE 5.7 packaged matrix for Editor, Development Game, Shipping Game, and UHT after the terrain/subgroup/representation pass.
- [x] Run `MassUnitSystem.Core.NativeMassLifecycle` from the packaged plugin: one pass, zero warnings, and zero errors.
- [x] Rebuild the real `GASDocumentationEditor Win64 DebugGame` target from a stopped editor.
- [x] Restart the editor on `/Game/Levels/Testing.Testing` and confirm MCP reconnects with the new DLL.
- [ ] Confirm the test spawner has `Use Navigation` enabled and is covered by a `NavMeshBoundsVolume`; press `P` to verify green navigable terrain.
- [x] In transient PIE, verify cubes traverse hills using navmesh elevation instead of preserving their original Z or clipping through terrain.
- [x] Verify one 25-unit crowd reports one managed subgroup, one ambient shared-path build, and zero queued per-unit path requests.
- [x] Trigger engagement, move the player across the terrain, and verify one shared engagement corridor, separation steering, follow slots, and attack requests.
- [ ] Confirm visual debug shows one cyan ambient subgroup corridor or one purple engagement corridor; per-unit red destination arrows remain off.
- [x] Confirm ending PIE leaves no dirty or transient content packages.

The saved Testing spawner still has `Use Navigation = false`; the terrain proof enabled it only on the transient PIE copy. Persist that one field deliberately when the team wants hill-following to be the default behavior of the host-project example.

## Terrain and environment proof

1. Add or resize a `NavMeshBoundsVolume` so it encloses every walkable hill, ramp, bridge, and destination.
2. Build navigation and inspect it with `P`. Gaps in green coverage are intentionally unreachable; direct fallback cannot infer terrain.
3. On the spawner enable `Use Navigation`, keep `Movement Mode = Planar 2D`, and keep `Conform To Navmesh Height` enabled.
4. Set `Navigation Height Offset` to the mesh pivot's height above the floor. The asset-free 100 cm cube uses 50 cm.
5. Start with `Managed Subgroup Size = 32`, `Subgroup Path Look Ahead Distance = 350 cm`, and separation enabled.
6. Test a path that crosses both rising and falling terrain, then a path around an obstacle. Verify units remain upright while their positions follow corridor Z.
7. Repeat with navigation temporarily unavailable. Units may use direct fallback, but the test must clearly document that terrain avoidance requires nav data.

## Custom static mesh proof

- [ ] Create `DA_UnitTemplate_Test` from `Unit Template`.
- [ ] Assign a project-owned static mesh with a sensible ground pivot; set skeletal/VAT fields empty for the first test.
- [ ] Assign the template to the Testing spawner and verify the ISM fallback replaces cubes without adding per-unit components.
- [ ] Verify transforms update in place: the topology revision must not change while units merely move.
- [ ] Validate the eight per-instance material floats in order: animation index, animation time, visual LOD, team id, team R, team G, team B, health percent.
- [ ] Apply damage to one represented unit and verify health custom data changes without replacing its Mass entity.

## VAT / vertex animation proof

- [ ] Prepare a project-owned VAT mesh, material, position/normal textures, and animation frame metadata. The repository intentionally ships no production art or baked VAT textures.
- [ ] Populate `Vertex Animation Texture` and `Animation Tags` on the unit template.
- [ ] Make the VAT material consume per-instance animation index/time and LOD, or configure a Niagara system that consumes the documented `Unit*` arrays.
- [ ] Verify Idle → Walk/Run → Attack → Death/Stun uses stable tag indices and does not use the `EMassUnitState` ordinal as an animation index.
- [ ] Test 500, 2,000, and target-platform maximum units; record game-thread, render-thread, GPU, instance count, and topology revision.

## Skeletal representation proof

- [ ] Assign a skeletal mesh plus Idle, Move, Attack, Death, and Stun clips on the template. An Animation Blueprint may be used when explicit clips are not assigned.
- [ ] Set project settings for `Max Skeletal Mesh Units`, `Skeletal Mesh Distance`, and `Skeletal Mesh Hysteresis`.
- [ ] Walk the player across the enter/exit boundary and verify the pool does not flicker or repeatedly fight the instanced representation.
- [ ] Verify the closest eligible units receive the bounded skeletal components and excess units remain instanced.
- [ ] Verify `Get Active Skeletal Mesh Count` never exceeds capacity and all components return to the pool after units leave range or die.
- [ ] Verify state changes select the expected clip and attack/death clips do not loop.

## Pooled sound and VFX proof

- [ ] Bind `On Crowd Cue Requested` once in a level/world manager.
- [ ] Route Movement Started, Ambient Interaction, Engagement Started/Ended, and Attack cues to pooled MetaSound/audio and Niagara systems.
- [ ] Use the supplied crowd-group handle, subgroup index, and world location; do not attach an audio or Niagara component to every entity.
- [ ] Confirm `Group Cue Cooldown` coalesces bursts from a subgroup and remains acceptable with hundreds of simultaneous attackers.
- [ ] Add project-owned cue assets only in the host project or a separate sample-content plugin, not the runtime plugin's required core.

## GAS end-to-end proof

- [ ] Ensure the player/target Actor owns a valid Ability System Component and initialized attributes.
- [ ] Create a small project Gameplay Effect for Mass-unit attacks and assign it to `Gameplay Effect To Target`, or use standard Actor damage—not both unless intentionally stacking them.
- [ ] Bind the project's hit/interaction trace to `Find Closest Spawned Unit` followed by `Damage Spawned Unit And Activate`.
- [ ] Verify the hit changes native Mass health, emits health/death events, and activates that unit's parent crowd against the instigator.
- [ ] Verify engaged Mass attacks apply the configured Gameplay Effect to the player ASC on authority and produce one Blueprint attack request per cooldown.
- [ ] Kill a cube/custom unit through the project GAS weapon path and verify death animation/state, query filtering, crowd cleanup, and representation cleanup.
- [ ] Test server/listen-server authority. Clients must not duplicate autonomous unit spawning, Actor damage, or Gameplay Effect application.
- [ ] Keep ASCs external by default. Use the optional `GAS Unit Integration` mapping only for the minority of units that genuinely require individual abilities/effects.

## Scale and regression gates

- [ ] 25-unit functional run: terrain, engagement, attack, damage, death, presentation cues, no warnings.
- [ ] 500-unit development run: stable frame pacing and bounded crowd/nav/visual work.
- [ ] 2,000-unit representation run: no per-unit Actor/component growth and no topology churn during motion.
- [ ] Target maximum run: tune `Max Crowd Units Per Update`, `Max Shared Path Builds Per Crowd Update`, visual interval, simulation LOD bands, subgroup size, visible distance, and skeletal capacity.
- [ ] Multiplayer smoke test: authority-only spawning/damage/effects and deterministic cleanup.
- [ ] Re-clone test: delete only the local plugin copy, clone upstream `main`, regenerate/rebuild, and repeat the 25-unit test without undocumented setup.

## Known boundaries

- A NavMesh is required for reliable hills and obstacle routing. Direct fallback is deliberately a cheap straight-line fallback and cannot discover terrain.
- Free3D is direct XYZ steering for flying/swimming/space behavior; standard Unreal navmesh is planar.
- Separation is a spatial-hash steering layer, not physical collision or full reciprocal-velocity obstacle avoidance.
- Shared corridor height and projected final slots keep instanced crowds terrain-aligned without per-unit ground traces. Exact skeletal foot contact on uneven ground remains a project animation/IK concern.
- The runtime plugin exposes asset-agnostic animation, material, cue, and GAS hooks; production meshes, VAT bakes, sounds, Niagara systems, Gameplay Effects, and abilities remain project-owned.
- Native Mass health is the scalable default. Giving every unit an Actor, ASC, skeletal component, audio component, or Behavior Tree defeats the intended architecture.

## Release and upstream

- [ ] Finish all compile, automation, host-build, and live Testing-level gates above.
- [ ] Review the final plugin-only diff and exclude unrelated documentation relocations or host-project changes.
- [ ] Commit the verified source/docs/tests, push the feature branch, open the PR, wait for checks, and merge to upstream `main`.
- [ ] Fresh-clone the merged plugin and repeat the documented setup before declaring the baseline release-ready.
