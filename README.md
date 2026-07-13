# Mass Unit System for Unreal Engine 5

Mass Unit System is a Blueprint- and C++-ready foundation for large groups of lightweight gameplay units using Unreal Engine's native Mass Entity framework.

Version **1.2.1** is verified with **Unreal Engine 5.7.2** on Windows for Editor, Development, and Shipping targets.

## What works out of the box

- Native, world-owned Mass entities—no Actor or UObject per unit
- A placeable **Mass Unit Spawner** with Blueprint-writable controls
- Visible engine-cube fallback with no project assets required
- Instanced static-mesh rendering for large unit counts
- Movement, targets, teams, lightweight combat, formations, and batched navigation
- Distance visibility/LOD controls and a bounded skeletal-mesh pool
- Optional Niagara, Gameplay Ability System, and Behavior Tree integrations
- Automatic world-subsystem initialization and deterministic cleanup

## 1. Install

Clone or copy the repository below the project's `Plugins` directory:

```powershell
Set-Location 'YourProject\Plugins'
git clone https://github.com/DigiLogicLabs/ue5-mass-units-plugin.git
```

```text
YourProject/
  Plugins/
    ue5-mass-units-plugin/
      MassUnitSystem.uplugin
```

For this project the expected descriptor is:

```text
MetalLegsGAS/Plugins/ue5-mass-units-plugin/MassUnitSystem.uplugin
```

Then:

1. Open the `.uproject`.
2. Allow Unreal to compile the plugin when prompted.
3. Open **Edit > Plugins > Gameplay** and confirm **Mass Unit System** is enabled.
4. Restart the editor after the first compile or any C++ plugin update.

The descriptor automatically enables the required engine plugins: `MassGameplay`, `Niagara`, and `GameplayAbilities`. No third-party plugin is required. A Blueprint-only host still needs an Unreal-supported C++ toolchain to compile this source plugin.

## 2. See 25 moving units in a level

This is the standard installation test. It needs no Data Asset, mesh, Niagara system, Mass Entity Config, or Unreal Mass Spawner.

1. Open a playable map, or choose **File > New Level > Basic**.
2. Open **Place Actors** and search for **Mass Unit Spawner**.
3. Drag **Mass Unit Spawner** onto the floor in the visible play area.
4. Leave `Unit Template` empty.
5. Press **Play**. Use **Simulate** for a truly empty level without a player pawn.

Default result:

- 25 native Mass entities spawn in a centered 5 x 5 grid.
- Engine cubes render through one stable dynamic-ISM fallback path.
- Units move 1,500 cm along the spawner's local positive X direction.
- Grid spacing is preserved while moving.
- Stopping play removes only the entities owned by that spawner.

If the grid is outside the camera, select the spawner and press **F** before Play to frame it. The cyan editor arrow shows the default movement direction. Set `Enable Visual Debug` on the spawner to draw cyan spawn boxes and green destination arrows.

## 3. Use your own unit mesh and stats

1. In the Content Browser choose **Add > Miscellaneous > Data Asset**.
2. Select `UnitTemplate`.
3. Name it, for example `DA_Unit_Infantry`.
4. Set health, damage, attack range/cooldown, move speed, team, and visual assets.
5. Assign the Data Asset to the placed spawner's `Unit Template` field.

Rendering selection is automatic:

1. Template static mesh
2. Project fallback static mesh
3. Engine cube

A skeletal mesh is used only at close range when skeletal-pool capacity is available. This keeps the large-unit path instanced by default.

## Blueprint setup

### Recommended: Blueprint subclass of the spawner

Create a Blueprint with parent class `MassUnitSpawner`. Its setup and optimization fields are Blueprint-writable:

- `Unit Template`, `Unit Count`
- `Columns`, `Unit Spacing`, `Spawn Height`
- `Spawn On Begin Play`, `Move On Begin Play`, `Destination Offset`
- `Use Navigation`, `Acceptance Radius`
- `Spawn On Authority Only`, `Destroy Spawned Units On End Play`
- `Enable Visual Debug`, `Debug Duration`

Useful Blueprint functions:

- `Spawn Units`
- `Move Spawned Units By Offset`
- `Command Spawned Units To Location`
- `Get Valid Spawned Units`
- `Get Valid Spawned Unit Count`
- `Destroy Spawned Units`

The spawner is only an ownership/bootstrap Actor. It does not tick and it does not represent each unit with an Actor.

### Direct subsystem pattern

Use this in a wave manager, encounter actor, Game Mode, or Level Blueprint:

```text
Event BeginPlay
  -> Get Mass Unit Subsystem
  -> Get Unit Manager
  -> Create Default Unit          (asset-free test)
     or Create Unit From Template (production data)
  -> store MassUnitHandle
  -> Set Unit Destination
```

Keep returned handles in the system that owns the wave. Before reusing a long-lived handle, call `Is Unit Valid`. Call `Destroy Unit` when that owner ends.

The subsystem is a `UTickableWorldSubsystem`; never construct it manually or add it to a Game Instance.

## Scale and optimization controls

Project-wide settings are under **Project Settings > Plugins > Mass Unit System**. Blueprints can read them through `Get Mass Unit System Settings`; per-spawner controls are Blueprint-readable and writable.

| Control | Default | Purpose |
|---|---:|---|
| `Max Units` | 10,000 | Hard safety cap for plugin-owned units in one world |
| `Visual Update Interval` | 0.033 s | ISM/Niagara upload rate; increase to reduce visual update cost |
| `LOD Distance Thresholds` | 500/1,500/3,000/6,000 cm | Distance bands written to unit visual LOD data |
| `Max Visible Distance` | 10,000 cm | Excludes farther units from visual submission; zero disables culling |
| `Max Skeletal Mesh Units` | 100 | Caps close-range skeletal components; set to zero for instanced-only use |
| `Skeletal Mesh Distance` | 300 cm | Distance inside which eligible units request skeletal representation |
| `Max Path Requests Per Frame` | 100 | Limits asynchronous navmesh requests submitted per world tick |
| `Enable Instanced Mesh Fallback` | On | Makes units visible without a custom Niagara system |
| `Fallback Static Mesh` | Empty | Optional project-wide mesh used when a template has no static mesh |
| `Default Niagara System` | Empty | Optional custom GPU renderer; dynamic ISM remains the zero-setup default |
| `Fallback To Direct Path` | On | Keeps movement functional when no navmesh data exists |

Blueprint diagnostics:

- `Get Unit Count`: number of valid plugin-owned Mass entities
- `Get Valid Spawned Unit Count`: valid units owned by one spawner
- `Is Using Niagara`: whether custom Niagara rendering is active
- `Get Instanced Mesh Component Count`: allocated fallback mesh groups
- `Get Instanced Mesh Instance Count`: currently submitted fallback instances
- `Get Instanced Mesh Topology Revision`: changes only when slots are added/removed; it remains stable while units move
- `Get Queued Request Count`: queued and in-flight navigation requests

Safe starting pattern:

1. Prove the install with 25 default cubes and direct movement.
2. Assign one static mesh and test 100 units.
3. Increase through 500 and 1,000 while profiling the target hardware.
4. Add navmesh routing, skeletal units, Niagara, GAS, or Behavior Trees one feature at a time.
5. Keep actor-backed GAS/Behavior Tree integration selective instead of attaching it to every entity.

`Max Units` is a safety cap, not a performance guarantee. Mesh complexity, materials, navigation, gameplay logic, platform, and camera coverage determine the real budget.

## Navigation and formations

Direct movement works without a Nav Mesh Bounds Volume.

For obstacle-aware movement:

1. Add a **Nav Mesh Bounds Volume**.
2. Build navigation and press **P** to verify the green navmesh.
3. Enable `Use Navigation` on the spawner, or call `Request Path` from Blueprint.

Formation shapes are `Rectangle`, `Line`, `Column`, `Wedge`, and `Circle`. Use the formation service when a group should retain assigned slots while moving.

## Verify the plugin

Open **Tools > Test Automation**, filter for `MassUnitSystem`, and run:

```text
MassUnitSystem.Core.NativeMassLifecycle
```

The test covers native entity creation, independent fragments, combat, path fallback, movement, formations, asset-free defaults, spawner ownership, stable ISM updates, destruction, and safe world teardown.

## Quick troubleshooting

**Mass Unit Spawner does not appear in Place Actors**

- Confirm the plugin compiled and is enabled.
- Restart the editor after adding/updating the source plugin.

**Unit count is zero**

- Spawn only from Game, PIE, or Game Preview worlds.
- Check `Max Units` and `Get Valid Spawned Unit Count`.

**Units exist but are invisible**

- Confirm a local player/spectator view exists.
- Check `Max Visible Distance` and `Enable Instanced Mesh Fallback`.
- Call `Get Instanced Mesh Instance Count`; a positive value means rendering data was submitted.

**Units do not move**

- Keep `Use Navigation` off for the first test.
- With navigation enabled, add/build a Nav Mesh Bounds Volume or keep direct-path fallback enabled.

More cases are covered in [docs/troubleshooting.md](docs/troubleshooting.md).

## Build the plugin in isolation

Set `UE_ENGINE_PATH` to the Unreal Engine root and run:

```powershell
$env:UE_ENGINE_PATH = 'D:\Unreal Engine\UE_5.7'
.\build_plugin.bat
```

```bash
UE_ENGINE_PATH=/path/to/UnrealEngine ./build_plugin.sh
```

The package build checks Editor Development, Game Development, Game Shipping, and Unreal Header Tool processing.

## More documentation

- [Five-minute quick start](docs/quick-start.md)
- [Setup](docs/setup.md)
- [Usage and integration](docs/usage.md)
- [Public API](docs/api.md)
- [Architecture](docs/architecture.md)
- [Troubleshooting](docs/troubleshooting.md)

## Intentional boundaries

- The repository does not bundle production art, VAT textures, or a Niagara asset.
- VAT authoring/decoding remains project-specific.
- Mass entities are not replicated as individual Actors; multiplayer representation/replication is project-specific.
- UE 5.7.2 is the validated baseline. Recompile and test for another engine minor version.

Copyright Digi Logic Labs LLC. All rights reserved. This repository currently contains no separate license grant.
