# Architecture

## Ownership

Each gameplay world owns Unreal's `UMassEntitySubsystem`. `UMassUnitSubsystem` declares that subsystem as a dependency and creates the plugin services for the same world. All unit handles ultimately identify entities in that native world-owned `FMassEntityManager`.

There is no fallback entity database and no shared static fragment storage. Every unit has independent fragment data in a Mass archetype.

```text
UWorld
  UMassEntitySubsystem
    FMassEntityManager
      native unit archetype/entities/fragments
  UMassUnitSubsystem
    entity facade
    navigation + formations
    timer-driven crowd decisions + spatial hash
    visual representations
    optional GAS + Behavior Tree bridges
```

## Simulation

Unit templates seed a common archetype. Movement, combat, and visibility are `UMassProcessor` classes registered with Mass phases. Processors iterate matching chunks and update fragments; gameplay code uses stable handles instead of actor pointers.

Continuous crowds split work by frequency. Smooth transform integration remains in the Mass movement processor. The world crowd service uses one timer, a round-robin unit budget, deterministic random streams, and a two-dimensional spatial hash for lower-frequency destination, separation, interaction, and behavior-LOD decisions. No controller, collision component, Behavior Tree, or Actor is allocated per ambient unit.

The manager keeps lightweight type/team indexes for convenient queries. Destroying an entity removes it from these indexes and invalidates the handle serial.

## Navigation and formations

Navigation batches requests, keeps a `PathId -> EntityHandle` map, and writes completed paths into the requesting entity's navigation fragment. Missing nav data can produce a direct path when configured.

Crowd sleep, interaction, pause, and unregister operations cancel queued and in-flight paths before clearing fragments, preventing superseded callbacks from reviving stopped movement.

The formation service owns formation membership and deterministic slot assignment. It writes formation targets into native entity fragments; the movement processor consumes those destinations.

## Representation

Simulation state remains in Mass. Representation is selected separately:

- Dynamic ISM is the zero-asset default so moving units can update stable instance slots without hierarchical tree rebuilds.
- A configured Niagara system receives packed arrays.
- A bounded pool supplies individual skeletal mesh components for close/high-detail units.

Distance visibility checks are staggered by current LOD and use the nearest local split-screen view. Crowd behavior LOD is separate: it uses all player-controller observers on authority, reduces decision frequency with distance, and can sleep ambient simulation beyond a group limit.

This separation avoids one actor per simulated unit. The vertex-animation registry maps animation gameplay tags to compact indices, while project-specific Niagara/material assets decode those indices.

## Selective UObject integrations

GAS and Behavior Trees are not appropriate for every lightweight unit. Their bridges allocate nothing per entity until a caller opts a unit in. GAS keeps ASC ownership outside the plugin; Behavior Trees create transient components only for selected handles.

## Shutdown

The native Mass subsystem owns entity-manager lifetime and tears down all entities with the world. Plugin services clear their indexes/components without dereferencing the Mass manager after it has begun shutdown. This ordering is covered by the native lifecycle automation test.
