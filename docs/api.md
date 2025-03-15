# API Documentation

## Classes
- `ULargeScaleUnitSystem`: Main class for managing the unit system.
- `UNiagaraUnitSimulator`: Handles Niagara-based simulation.
- `UPathingManager`: Manages group pathing and swarm behaviors.

## Functions
- `SpawnUnitSwarm(int Count, FVector Location, ...)`: Spawns a swarm of units.
- `SetUnitModel(UStaticMesh* Mesh)`: Sets the mesh for units.
- `AssignGASAbility(UClass* AbilityClass)`: Assigns a GAS ability to units directly via Unreal Engine's **Gameplay Ability System**.

## Variables
- `MaxSkeletalUnits`: Controls the maximum number of active skeletal units.
- `LODSettings`: Array of LOD settings for units.