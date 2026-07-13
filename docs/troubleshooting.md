# Troubleshooting

## The plugin does not compile

- Verify the engine is UE 5.7 and the installed C++ toolchain is accepted by that engine.
- Confirm `MassUnitSystem.uplugin` is directly below one folder in `<Project>/Plugins`.
- Regenerate project files after cloning.
- Remove only the plugin's generated `Binaries` and `Intermediate` directories before rebuilding; do not remove source or project content.
- Use `build_plugin.bat` or `build_plugin.sh` to expose Editor, Game, Shipping, and UHT failures in isolation.

## The subsystem is null

Resolve it from an object in a Game, PIE, or Game Preview world. It intentionally does not exist in editor asset worlds or as a Game Instance subsystem.

## Unit creation fails

- Pass a valid `UnitTemplate` Data Asset.
- For an asset-free control case, call `Create Default Unit` or leave the `Mass Unit Spawner` template empty.
- Check `Is Unit Valid` on the returned handle.
- Check whether **Max Units** has been reached.
- Look for `LogMassUnitSystem` warnings.

## Units exist but are not visible

- Ensure **Enable Instanced Mesh Fallback** is on, or assign a valid Niagara system.
- A template static mesh is preferred; otherwise configure a fallback mesh. If both are empty, the engine cube should appear.
- Check maximum visible distance and LOD settings.
- Ensure the unit is spawned in a gameplay world and within the camera range.
- Query `Get Instanced Mesh Instance Count`. A positive value confirms fallback instances were submitted and usually means the camera, distance, or level placement is the remaining issue.

## Units do not move

- Call either `Set Unit Destination` or `Request Path` on the same valid handle.
- With obstacle-aware navigation, add/build a Nav Mesh Bounds Volume.
- If no nav data exists, enable **Fallback To Direct Path**.
- Dead or stunned units intentionally do not move.

## Instanced units flash or blink while moving

- Use plugin version 1.2.1 or newer. Earlier fallback rendering cleared and rebuilt HISM instances on each visual refresh.
- Confirm `Get Instanced Mesh Instance Count` stays equal to the visible unit count.
- `Get Instanced Mesh Topology Revision` should remain stable during movement; a rising value means units are repeatedly changing mesh groups or visibility membership.

## Combat does not apply damage

- Source and target must have different team IDs.
- Assign the target with `Set Unit Target`.
- Ensure the target is within attack range and the attack cooldown can elapse.
- Lightweight combat does not require an ASC.

## GAS calls fail

Register a valid externally owned ASC for the unit first. Ability/effect calls return false when the unit, ASC, class, or gameplay tag is invalid. The plugin does not create an ASC automatically.

## Headless automation crashes before tests start

Some installed UE 5.7.2 configurations fail on their first tick with `-NullRHI`. Use `-RenderOffscreen` with a real RHI. Do not add `-NoShaderCompile` to a fresh minimal host, because default editor materials may still require compilation.

## Useful test

Run `MassUnitSystem.Core.NativeMassLifecycle` in Unreal's Automation window. It is a fast functional test rather than a performance benchmark.
