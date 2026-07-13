# Mass Unit System for Unreal Engine 5

Mass Unit System is a source plugin for building lightweight gameplay units on Unreal Engine's native Mass Entity framework. Version 1.1.0 is verified with Unreal Engine 5.7.2 on Windows for Editor, Development, and Shipping targets.

The plugin works without bundled art assets: units use their template's static mesh when supplied and otherwise fall back to an instanced engine cube. A project can opt into a custom Niagara system, skeletal representations, Gameplay Ability System components, or Behavior Trees as needed.

## What works

- Native, world-owned `FMassEntityHandle` entities and per-entity Mass fragments
- Blueprint and C++ unit creation, destruction, lookup, transforms, destinations, targets, teams, and damage
- Auto-registered Mass movement, combat, and visibility processors
- Batched asynchronous navmesh paths with a configurable direct-path fallback
- Rectangle, line, column, wedge, and circle formation slots and formation movement
- Asset-free hierarchical instanced static-mesh rendering
- Optional Niagara array upload and proximity-limited skeletal mesh pooling
- Optional bridge to externally owned Ability System Components
- Optional Behavior Tree/Blackboard bridge for a deliberately small subset of units
- Native lifecycle automation coverage in the editor module

## Install

Clone or copy the repository anywhere under your project's `Plugins` directory. The folder name does not need to match the plugin name.

```text
YourProject/
  Plugins/
    ue5-mass-units-plugin/
      MassUnitSystem.uplugin
```

Open the project and allow Unreal to compile the plugin. The descriptor enables the required engine plugins (`MassGameplay`, `Niagara`, and `GameplayAbilities`) and enables Mass Unit System by default. A source build requires a C++ toolchain supported by your Unreal Engine installation.

The source descriptor is intentionally not pinned to one engine patch version. UE 5.7.2 is the validated baseline; compile and test before using another engine minor version.

## First unit

1. In the Content Browser, create a **Data Asset** of class `UnitTemplate`.
2. Set health, damage, speed, team, and an optional static or skeletal mesh.
3. In a gameplay Blueprint, call `Get Mass Unit Subsystem` with a world-context object.
4. Call `Get Unit Manager`, then `Create Unit From Template`.
5. Send the returned `MassUnitHandle` to `Set Unit Destination`, `Request Path`, formation, targeting, or damage functions.

The subsystem is a `UTickableWorldSubsystem`; projects do not create or manually initialize it.

```cpp
#include "Core/MassUnitSubsystem.h"
#include "Entity/MassUnitEntityManager.h"
#include "Entity/UnitTemplate.h"

if (UMassUnitSubsystem* Units = UMassUnitSubsystem::Get(this))
{
    const FMassUnitHandle Unit = Units->GetUnitManager()->CreateUnitFromTemplate(
        UnitTemplate,
        FTransform(SpawnLocation));

    Units->GetUnitManager()->SetUnitDestination(Unit, Destination, 50.0f);
}
```

## Optional integrations

GAS remains actor/ASC-owned by design. Register an existing `UAbilitySystemComponent` for only the Mass units that need full GAS behavior, then use the bridge to grant, activate, or apply effects. The plugin never creates one actor and ASC per Mass entity.

For custom Niagara rendering, assign a system in **Project Settings > Plugins > Mass Unit System**. The expected user parameters are documented in [docs/usage.md](docs/usage.md). With no Niagara asset configured, instanced rendering remains functional.

## Build and test

Set `UE_ENGINE_PATH` to the Unreal Engine root, then run:

```powershell
.\build_plugin.bat
```

```bash
UE_ENGINE_PATH=/path/to/UnrealEngine ./build_plugin.sh
```

Packaged output defaults to `Build/Package`. Optional arguments select output directory, target platform, and configuration; see [docs/setup.md](docs/setup.md).

The editor automation test is named:

```text
MassUnitSystem.Core.NativeMassLifecycle
```

## Documentation

- [Setup](docs/setup.md)
- [Usage and integration](docs/usage.md)
- [Public API](docs/api.md)
- [Architecture](docs/architecture.md)
- [Troubleshooting](docs/troubleshooting.md)
- [Out-of-the-box readiness](OutOfTheBoxReadinessReport.md)

## Current boundaries

- The repository does not include production art, VAT textures, or a Niagara asset.
- The vertex-animation manager maps animation tags/indices; authoring and decoding VAT assets remains project-specific.
- Mass entities are intentionally not replicated as individual actors. Add a project-specific replication strategy when multiplayer requires it.
- Validate unit-count and rendering budgets on target hardware; the configured `MaxUnits` value is a safety cap, not a performance guarantee.

Copyright Digi Logic Labs LLC. All rights reserved. This repository currently contains no separate license grant.
