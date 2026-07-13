# Setup

## Requirements

- Unreal Engine 5.7 (validated with 5.7.2)
- A C++ compiler/toolchain supported by that engine installation
- A C++ project, or a Blueprint project that can compile a source plugin

No third-party plugin is required. `MassGameplay`, `Niagara`, and `GameplayAbilities` are engine plugins enabled by `MassUnitSystem.uplugin`.

## Project installation

1. Put this repository under `<Project>/Plugins/`. Its directory name can be anything.
2. Confirm `MassUnitSystem.uplugin` is directly inside that directory.
3. Regenerate project files if your IDE does not discover the modules automatically.
4. Open the project and accept Unreal's compile prompt.
5. Confirm **Edit > Plugins > Gameplay > Mass Unit System** is enabled.

The world subsystem initializes itself in Game, PIE, and Game Preview worlds. Do not add it to a Game Instance or construct it manually. For the shortest visible validation, continue with the [five-minute quick start](quick-start.md).

## Project settings

Open **Project Settings > Plugins > Mass Unit System** to configure:

- `Max Units`: creation safety cap, default 10,000
- `Max Skeletal Mesh Units`: component pool capacity, default 100
- `Visual Update Interval`: Niagara/ISM upload interval
- LOD thresholds, skeletal range, and maximum visible range
- Optional default Niagara system and fallback static mesh
- Direct-path behavior when no nav data exists

Defaults require no assets. If neither a unit template nor the project setting supplies a static mesh, the engine cube is instanced so spawned units are visible.

## Navigation

Add a Nav Mesh Bounds Volume and build navigation for obstacle-aware paths. When no nav data exists, `RequestPath` uses a direct destination by default. Disable `bFallbackToDirectPath` when missing navigation should be treated as an error.

## Build a distributable package

Set `UE_ENGINE_PATH` to the engine root—the directory containing `Engine/Build/BatchFiles`—and run the platform helper.

```powershell
$env:UE_ENGINE_PATH = 'D:\Unreal Engine\UE_5.7'
.\build_plugin.bat [OutputDirectory] [TargetPlatform] [Configuration]
```

```bash
UE_ENGINE_PATH=/opt/UnrealEngine ./build_plugin.sh [output-directory] [target-platform] [configuration]
```

Defaults are `Build/Package`, `Win64` on Windows or the current Unix host platform, and `Development`. Both helpers call UAT `BuildPlugin`, which also checks Unreal Header Tool and non-editor targets.

## Smoke test

After compilation, run **Tools > Test Automation**, filter for `MassUnitSystem`, and execute:

```text
MassUnitSystem.Core.NativeMassLifecycle
```

The test creates two native Mass entities and checks independent fragments, combat, path fallback, movement, formation membership, destruction, and safe world teardown.
