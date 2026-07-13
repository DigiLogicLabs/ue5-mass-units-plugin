# Five-minute quick start

This path starts from a normal playable level and requires no project assets, Mass Spawner, Entity Config, Niagara system, or per-unit Actors. It intentionally uses the plugin's engine-cube fallback so installation problems are easy to separate from content problems.

## 1. Confirm the plugin loaded

1. Put the repository below `<Project>/Plugins/`.
2. Enable **Mass Unit System** under **Edit > Plugins > Gameplay** if it is not already enabled.
3. Compile and restart the editor after installing or updating the source plugin.
4. Optionally run **Tools > Test Automation** and execute `MassUnitSystem.Core.NativeMassLifecycle`.

The descriptor enables the required engine plugins. You do not need to add a Mass Entity Config asset or place Unreal's standard Mass Spawner for this plugin-owned workflow.

## 2. Place the zero-asset spawner

1. Open a level that has a local player/camera. A new **Basic** or template level is ideal. In a truly empty level, use **Simulate** so the editor spectator supplies a view.
2. In **Place Actors**, search for **Mass Unit Spawner**.
3. Drag it into the visible play area and keep it above the floor. The default `Spawn Height` of 50 cm already centers the fallback cube on a floor at the actor's Z position.
4. Leave `Unit Template` empty for the first run.
5. Press **Play** or **Simulate**.

Expected result:

- 25 native Mass entities appear as a centered 5 x 5 grid of engine cubes.
- They move 1,500 cm along the spawner actor's local positive X direction while preserving spacing.
- The spawner actor does not tick and does not create one Actor per unit.
- Stopping play destroys only the Mass entities owned by that spawner.

If the units are outside the camera, select the spawner before starting and use **F** to frame it in the editor viewport. The cyan arrow shows the default movement direction. Enable `Visual Debug` on the spawner to draw cyan spawn boxes and green command arrows during play.

## 3. Make it data-driven

1. In the Content Browser, choose **Add > Miscellaneous > Data Asset**.
2. Select `UnitTemplate` and name the asset, for example `DA_Unit_Infantry`.
3. Set `Move Speed`, health, damage, team, and an optional static or skeletal mesh.
4. Assign the asset to the spawner's `Unit Template` field.

Static meshes use the instanced fallback automatically. A template with no static mesh still uses the project fallback mesh or engine cube. Skeletal meshes are used only inside the configured skeletal distance and bounded component-pool capacity.

## 4. Add obstacle-aware movement only when needed

The default spawner writes direct destinations, which is the most deterministic first-run check.

For navmesh routing:

1. Add a **Nav Mesh Bounds Volume** around the traversable area.
2. Build navigation and press **P** to confirm the green navmesh overlay.
3. Enable `Use Navigation` on the Mass Unit Spawner.

If navigation data is absent, the default project setting falls back to a direct path. Disable **Project Settings > Plugins > Mass Unit System > Fallback To Direct Path** when missing navigation should fail loudly instead.

## Blueprint-only pattern

Use this when an encounter, Game Mode, or Level Blueprint owns spawning instead of a placed spawner:

```text
Event BeginPlay
  -> Get Mass Unit Subsystem
  -> Get Unit Manager
  -> Create Default Unit          (no asset required)
     or Create Unit From Template (production data asset)
  -> retain MassUnitHandle
  -> Set Unit Destination
```

Store handles in an array owned by the encounter/wave system. Validate long-lived handles with `Is Unit Valid`, and call `Destroy Unit` when that owner is retired. Do not create a UObject or Actor per Mass entity.

## Repeatable production pattern

- Use one `UnitTemplate` Data Asset per reusable unit archetype.
- Use one `Mass Unit Spawner` or project-owned wave manager per ownership/lifetime boundary.
- Spawn on authority by default. The built-in spawner's `Spawn On Authority Only` prevents accidental duplicate simulation; Mass replication remains project-specific.
- Start with direct movement, then enable navmesh routing after the visual/spawn baseline passes.
- Use the formation service for group destinations instead of issuing converging point targets when spacing matters.
- Keep optional GAS and Behavior Tree bridges selective; register only units that need actor-backed services.
- Scale in measured steps such as 25, 100, 500, and 1,000 units while profiling the target platform.

## Fast diagnostics

At runtime, these Blueprint calls separate common failure modes:

- `Get Valid Spawned Unit Count` on the spawner: ownership and creation succeeded.
- `Get Unit Count` on the unit manager: world subsystem and native Mass creation succeeded.
- `Is Using Niagara` on the visual system: reports whether a configured Niagara asset owns rendering.
- `Get Instanced Mesh Instance Count`: reports the asset-free/static-mesh fallback instance count.
- `Get Queued Request Count` on the navigation system: exposes queued or in-flight path requests.

If unit counts are correct but nothing is visible, verify there is a local player view, the spawner is within `Max Visible Distance`, and **Enable Instanced Mesh Fallback** is enabled.
