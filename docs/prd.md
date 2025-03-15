# Product Requirements Document (PRD)

## Introduction
This plugin provides a highly optimized solution for managing large-scale unit systems in Unreal Engine 5.5. It leverages the **Gameplay Ability System (GAS)** for AI behaviors and abilities, making it ideal for games with massive enemy swarms, such as roguelikes or strategy games.

## Features
- **Large-Scale Unit Simulation**: Utilizes Niagara to simulate thousands of units with minimal CPU overhead.
- **Efficient Animations**: Employs vertex animations for unit movements and actions, reducing performance impact.
- **Dynamic Pathing**: Implements group-based pathing and swarm behaviors for realistic and efficient unit movement.
- **GAS Integration**: Supports AI behaviors and abilities directly via Unreal Engine's Gameplay Ability System.
- **Procedural Dungeon Compatibility**: Adapts to dynamic environments, ensuring units can navigate procedurally generated levels.
- **Interactive Units**: Switches to skeletal meshes and active ragdolls when the player interacts with units, enhancing immersion.
- **Performance Optimizations**: Includes LODs, instanced rendering, and culling to maintain high frame rates.

## Requirements
- Unreal Engine 5.5 or later.
- Basic knowledge of Unreal Engine’s Niagara, navigation systems, and **Gameplay Ability System (GAS)**.