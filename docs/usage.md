# Usage Guide

## Spawning Units
- Use the `SpawnUnitSwarm` function in Blueprint or C++ to create a swarm of units.
- Configure parameters like unit count, spawn area, and behavior settings.

## Customizing Units
- Assign custom meshes and materials to the unit actors.
- Provide vertex animation textures for different actions.
- Define GAS abilities for units to use directly via Unreal Engine's **Gameplay Ability System**.

## Managing Interactions
- The plugin automatically handles switching to skeletal meshes on interaction.
- Adjust the pooling settings to control how many skeletal units can be active at once.

## Best Practices
- Keep unit counts reasonable for your target hardware.
- Use LODs effectively to reduce detail at distance.
- Minimize the number of units using GAS at any given time.