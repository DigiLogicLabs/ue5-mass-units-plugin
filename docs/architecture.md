# Architecture

## Overview
The plugin is structured to maximize performance and modularity. It consists of several key components:

1. **Niagara Simulation**: Handles the position, movement, and basic behaviors of units.
2. **Vertex Animation System**: Manages efficient animations for units using texture-based animations.
3. **Pathing Module**: Provides group-based pathing and swarm behaviors, leveraging Unreal Engine’s navigation system.
4. **GAS Integration**: Connects units directly to Unreal Engine's **Gameplay Ability System** for advanced AI and interactions.
5. **Interaction Handler**: Manages the transition to skeletal meshes and ragdolls when units are interacted with.
6. **Optimization Tools**: Implements LODs, instanced rendering, and culling to ensure high performance.

## Component Interaction
- The Niagara system simulates unit positions and states.
- The pathing module updates group paths and influences Niagara simulations.
- When interaction occurs, the interaction handler swaps Niagara particles for skeletal actors.
- GAS is used selectively for units that require complex behaviors, with optimizations to limit overhead.

## Design Decisions
- **Niagara for Simulation**: Chosen for its GPU efficiency and ability to handle large numbers of entities.
- **Vertex Animations**: Selected to reduce CPU load compared to traditional skeletal animations.
- **Group Pathing**: Implemented to minimize pathfinding calculations by treating groups as single entities.
- **Pooling for Skeletal Units**: Used to cap the number of active skeletal meshes and prevent performance spikes.
- **Direct GAS Integration**: Leverages Unreal Engine's built-in Gameplay Ability System for flexibility and standardization.