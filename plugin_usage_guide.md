# UE5 Large-Scale Unit System Plugin Usage Guide

## Introduction

This guide provides instructions for using the UE5 Large-Scale Unit System Plugin in your Unreal Engine 5.5 project. The plugin enables the rendering and simulation of thousands of units on screen simultaneously, with seamless integration with the Gameplay Ability System (GAS) and GASCompanion.

## Prerequisites

Before using this plugin, ensure you have:

- Unreal Engine 5.5 or later
- GASCompanion Plugin installed and configured
- Basic knowledge of Unreal Engine's Mass Entity System, Niagara, and GAS

## Installation

1. Copy the `MassUnitSystem` folder to your project's `Plugins` directory
2. Restart the Unreal Editor
3. Enable the plugin via Edit > Plugins > Gameplay > Mass Unit System
4. Restart the editor when prompted

## Quick Start

### Creating Your First Unit Swarm

1. **Create a Unit Template**:
   - In the Content Browser, right-click and select Create > Mass Unit System > Unit Template
   - Configure the template with appropriate mesh, animations, and attributes
   - Save the template as `BP_YourUnitTemplate`

2. **Set Up a Spawner**:
   - Add a `MassUnitSpawner` actor to your level
   - Set the Unit Template to your created template
   - Configure spawn parameters (count, area, formation)
   - Set team and faction information

3. **Configure Navigation**:
   - Ensure your level has a NavMesh
   - Add a `MassUnitNavigationVolume` to define movement areas
   - Configure obstacle layers and avoidance settings

4. **Run the Simulation**:
   - Press Play to see your units in action
   - Units will use vertex animation for efficient rendering
   - When player approaches, units will transition to skeletal meshes as needed

## Core Components

### Unit Templates

Unit Templates define the properties of unit types. To create effective templates:

1. **Visual Setup**:
   - Assign appropriate skeletal and static meshes
   - Configure vertex animation textures
   - Set up LOD transitions

2. **Gameplay Setup**:
   - Define base attributes (health, damage, speed)
   - Assign default abilities
   - Configure team and faction information

3. **Behavior Setup**:
   - Select default behavior profile
   - Configure formation preferences
   - Set up combat parameters

### Formation System

The Formation System manages how units move and position themselves as groups:

1. **Creating Formations**:
   - Use `UFormationSystem::CreateFormation()` to create a formation
   - Specify formation type and units to include
   - Get a formation handle for future updates

2. **Updating Formations**:
   - Use `UFormationSystem::UpdateFormation()` to move formations
   - Provide destination and orientation
   - Formation will maintain integrity during movement

3. **Custom Formations**:
   - Create a new `UFormationTemplate` subclass
   - Implement `CalculatePositions()` method
   - Register template with the formation system

### GAS Integration

The plugin integrates with GAS and GASCompanion for ability and attribute management:

1. **Ability Setup**:
   - Create abilities using GASCompanion's workflow
   - Assign abilities to unit templates
   - Configure ability activation conditions

2. **Attribute Management**:
   - Define attributes using GASCompanion's attribute sets
   - Configure base values in unit templates
   - Use gameplay effects for attribute modifications

3. **AI Integration**:
   - Use `GSCUnitBehaviorTree` tasks for AI behavior
   - Configure behavior trees for different unit types
   - Set up perception and decision making

## Advanced Usage

### Performance Optimization

To achieve maximum performance with large numbers of units:

1. **LOD Configuration**:
   - Configure LOD distances in plugin settings
   - Set up appropriate LOD transitions
   - Use simplified behaviors for distant units

2. **Batching and Instancing**:
   - Group similar units for batch processing
   - Use instanced rendering for similar units
   - Share resources between units

3. **Spatial Partitioning**:
   - Configure spatial grid settings
   - Adjust update frequencies based on distance
   - Use visibility culling for off-screen units

### Transition System

The transition system handles switching between vertex animation and skeletal meshes:

1. **Transition Triggers**:
   - Configure distance-based triggers
   - Set up interaction-based triggers
   - Create custom triggers for specific gameplay events

2. **Transition Settings**:
   - Adjust transition blend times
   - Configure maximum simultaneous transitions
   - Set priority for transition requests

3. **Custom Transition Logic**:
   - Implement `IUnitTransitionHandler` interface
   - Override transition decision logic
   - Create custom transition effects

### Custom Unit Behaviors

To create custom unit behaviors:

1. **Behavior Profiles**:
   - Create a new `UUnitBehaviorProfile` asset
   - Configure behavior parameters
   - Assign to unit templates

2. **Custom Processors**:
   - Create a new Mass Processor subclass
   - Implement processing logic
   - Register processor with the Mass Entity System

3. **Behavior Trees**:
   - Create custom behavior tree tasks
   - Implement GASCompanion integration
   - Configure decision making and responses

## Integration with Existing Projects

### Working with Procedural Dungeons

To integrate with procedural dungeon generation:

1. **Dynamic Navigation**:
   - Call `UMassUnitNavigationSystem::UpdateNavigationData()` when environment changes
   - Configure dynamic obstacle avoidance
   - Set up path recalculation triggers

2. **Environment Adaptation**:
   - Use environment-aware formations
   - Configure space requirements for units
   - Implement dynamic formation adaptation

3. **Visibility and Culling**:
   - Set up occlusion culling for dungeon environments
   - Configure visibility checks for units
   - Optimize updates based on room boundaries

### Roguelike Element Integration

To integrate with roguelike progression systems:

1. **Unit Progression**:
   - Implement level-up mechanics for units
   - Configure attribute scaling with level
   - Set up ability unlocks based on progression

2. **Procedural Unit Generation**:
   - Create unit variation system
   - Configure random attribute ranges
   - Set up equipment and ability randomization

3. **Loot and Rewards**:
   - Implement loot drop system for units
   - Configure reward distribution
   - Set up progression-based rewards

## Troubleshooting

### Common Issues

1. **Performance Problems**:
   - Check LOD configuration
   - Verify batching and instancing settings
   - Profile to identify bottlenecks

2. **Visual Glitches**:
   - Check vertex animation textures
   - Verify transition settings
   - Ensure meshes have proper LODs

3. **Navigation Issues**:
   - Verify NavMesh generation
   - Check path request batching
   - Ensure proper obstacle setup

### Debugging Tools

The plugin includes several debugging tools:

1. **Visual Debugging**:
   - Enable "Show Mass Unit Debug" in viewport options
   - Use different colors for states and teams
   - Visualize paths and formations

2. **Performance Monitoring**:
   - Use "Mass Unit Performance Stats" command
   - Monitor batch processing times
   - Track memory usage and allocations

3. **Logging**:
   - Configure log categories in project settings
   - Use "MassUnit" log category for specific issues
   - Enable verbose logging for detailed information

## Best Practices

1. **Start Small**:
   - Begin with a small number of units
   - Gradually increase as you optimize
   - Test with different unit types and behaviors

2. **Optimize Early**:
   - Profile regularly during development
   - Address performance issues immediately
   - Consider performance implications of customizations

3. **Use Templates**:
   - Create reusable unit templates
   - Share resources between similar units
   - Use inheritance for specialized units

4. **Test on Target Hardware**:
   - Verify performance on minimum spec hardware
   - Test with different hardware configurations
   - Optimize for your target platform

## API Reference

For detailed API documentation, refer to the Technical Specification document and the in-code documentation.

## Conclusion

The UE5 Large-Scale Unit System Plugin provides a powerful foundation for creating games with thousands of units. By following this guide, you can effectively integrate and utilize the plugin in your Unreal Engine 5.5 project with GASCompanion.

For additional support, refer to the example projects and the implementation roadmap for future enhancements.
