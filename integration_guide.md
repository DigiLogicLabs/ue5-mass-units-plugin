# Mass Unit System Plugin: Integration Guide

This guide provides detailed steps for integrating the Mass Unit System plugin into your existing Unreal Engine 5.5 project. The plugin enables large-scale unit simulation with Gameplay Ability System (GAS) integration, making it ideal for games with massive enemy swarms.

## Prerequisites

Before integrating the plugin, ensure you have:

- Unreal Engine 5.5 or later installed
- An existing UE5 project
- Basic knowledge of Unreal Engine Blueprint and C++ (for advanced customization)

## Step 1: Install the Plugin

### Option 1: Clone Directly into Your Project (Recommended)

1. Create a `Plugins` directory in your project root if it doesn't already exist
2. Clone the repository directly into your Plugins folder:
   ```
   cd YourProject/Plugins
   git clone https://github.com/DigiLogicLabs/ue5-mass-units-plugin.git MassUnitSystem
   ```
3. Right-click on your .uproject file and select "Generate Visual Studio project files"
4. Open your project in Unreal Engine
5. The plugin should be automatically detected and compiled

### Option 2: Download and Copy

1. Download the plugin repository as a ZIP file
2. Extract it into your project's `Plugins` directory, renaming it to "MassUnitSystem"
3. Right-click on your .uproject file and select "Generate Visual Studio project files"
4. Open your project in Unreal Engine
5. The plugin should be automatically detected and compiled

### Option 3: Build from Source

1. Clone or download the plugin repository
2. Navigate to the plugin directory
3. Update the `UE_PATH` variable in `build_plugin.bat` to point to your Unreal Engine installation
4. Run `build_plugin.bat` to build the plugin
5. Copy the built plugin to your project's `Plugins` directory

## Step 2: Enable Required Dependencies

The Mass Unit System plugin depends on several other plugins. Make sure they are enabled in your project:

1. Open your project in Unreal Engine
2. Go to Edit > Plugins
3. Enable the following plugins:
   - Mass Entity
   - Mass Gameplay
   - Niagara
   - Gameplay Abilities
   - GASCompanion (optional but recommended)
4. Restart the editor when prompted

## Step 3: Configure Project Settings

### Navigation Settings

1. Open Project Settings > Navigation Mesh
2. Configure the following settings:
   - Cell Size: 10.0
   - Cell Height: 10.0
   - Agent Radius: 35.0
   - Agent Height: 200.0
   - Enable "Generate Navigation Only Around Navigation Invokers" for better performance

### Collision Settings

1. Open Project Settings > Engine > Collision
2. Create a new collision profile named "MassUnit" with:
   - Collision Enabled: Query Only
   - Object Type: Pawn
   - Custom responses for appropriate channels (typically block WorldStatic, overlap Pawns)

### Input Mappings (Optional)

If you plan to control units directly:

1. Open Project Settings > Engine > Input
2. Add appropriate action mappings for unit selection and commands

## Step 4: Initialize the Mass Unit Subsystem

The Mass Unit Subsystem is the central component that manages all unit-related operations. You need to access it from your game code.

### Blueprint Implementation

1. Create a new Blueprint class based on your game mode or game instance
2. Add the following function to get the Mass Unit Subsystem:

```
UFUNCTION(BlueprintCallable, Category = "Mass Unit System")
UMassUnitSubsystem* GetMassUnitSubsystem()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        return GameInstance->GetSubsystem<UMassUnitSubsystem>();
    }
    return nullptr;
}
```

3. Use this function to access the subsystem and its components

### C++ Implementation

1. Include the necessary headers in your game code:

```cpp
#include "Core/MassUnitSubsystem.h"
#include "Entity/MassUnitEntityManager.h"
#include "Gameplay/GASUnitIntegration.h"
```

2. Get the subsystem from your game instance:

```cpp
UMassUnitSubsystem* MassUnitSubsystem = GetGameInstance()->GetSubsystem<UMassUnitSubsystem>();
```

3. Access the various components through the subsystem:

```cpp
UMassUnitEntityManager* UnitManager = MassUnitSubsystem->GetUnitManager();
UFormationSystem* FormationSystem = MassUnitSubsystem->GetFormationSystem();
UMassUnitNavigationSystem* NavigationSystem = MassUnitSubsystem->GetNavigationSystem();
```

## Step 5: Create Unit Templates

Unit templates define the properties and behaviors of different unit types.

### Blueprint Implementation

1. Create a new Blueprint class based on UUnitTemplate
2. Configure the template properties:
   - Unit Type: Set appropriate gameplay tag (e.g., "Unit.Infantry")
   - Base Stats: Set health, damage, move speed
   - Visual: Assign skeletal mesh, static mesh, and textures
   - Team: Set team ID and color
   - Formation: Set default formation type

### C++ Implementation

1. Create a new C++ class derived from UUnitTemplate
2. Override the necessary properties and methods
3. Register the template with the unit manager

## Step 6: Spawn Units

Once you have unit templates, you can spawn units in your game.

### Blueprint Implementation

1. Get the Mass Unit Subsystem
2. Get the Unit Manager from the subsystem
3. Call CreateUnitFromTemplate with your template and spawn transform:

```
UMassUnitSubsystem* Subsystem = GetMassUnitSubsystem();
if (Subsystem)
{
    UMassUnitEntityManager* UnitManager = Subsystem->GetUnitManager();
    if (UnitManager)
    {
        FTransform SpawnTransform = GetActorTransform();
        FMassUnitHandle UnitHandle = UnitManager->CreateUnitFromTemplate(UnitTemplate, SpawnTransform);
    }
}
```

### C++ Implementation

```cpp
UMassUnitSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMassUnitSubsystem>();
if (Subsystem)
{
    UMassUnitEntityManager* UnitManager = Subsystem->GetUnitManager();
    if (UnitManager)
    {
        FTransform SpawnTransform = GetActorTransform();
        FMassUnitHandle UnitHandle = UnitManager->CreateUnitFromTemplate(UnitTemplate, SpawnTransform);
    }
}
```

## Step 7: Organize Units in Formations

The Formation System allows you to organize units into various formations.

### Blueprint Implementation

1. Get the Formation System from the Mass Unit Subsystem
2. Create a formation:

```
UFormationSystem* FormationSystem = Subsystem->GetFormationSystem();
if (FormationSystem)
{
    int32 FormationHandle = FormationSystem->CreateFormation(Location, Rotation, "Infantry");
    
    // Add units to the formation
    for (FMassUnitHandle UnitHandle : UnitHandles)
    {
        FormationSystem->AddEntityToFormation(UnitHandle, FormationHandle);
    }
    
    // Set formation target
    FormationSystem->SetFormationTarget(FormationHandle, TargetLocation);
}
```

### C++ Implementation

```cpp
UFormationSystem* FormationSystem = Subsystem->GetFormationSystem();
if (FormationSystem)
{
    int32 FormationHandle = FormationSystem->CreateFormation(Location, Rotation, "Infantry");
    
    // Add units to the formation
    for (const FMassUnitHandle& UnitHandle : UnitHandles)
    {
        FormationSystem->AddEntityToFormation(UnitHandle, FormationHandle);
    }
    
    // Set formation target
    FormationSystem->SetFormationTarget(FormationHandle, TargetLocation);
}
```

## Step 8: Implement GAS Integration

The GAS Integration system allows units to use Gameplay Abilities.

### Blueprint Implementation

1. Get the GAS Integration from the Mass Unit Subsystem
2. Grant abilities to units:

```
UGASUnitIntegration* GASIntegration = Subsystem->GetGASIntegration();
if (GASIntegration)
{
    // Grant ability to unit
    GASIntegration->GrantAbility(UnitHandle, AbilityClass, 1);
    
    // Activate ability
    GASIntegration->ActivateAbility(UnitHandle, AbilityTag);
}
```

### C++ Implementation

```cpp
UGASUnitIntegration* GASIntegration = Subsystem->GetGASIntegration();
if (GASIntegration)
{
    // Grant ability to unit
    GASIntegration->GrantAbility(UnitHandle, AbilityClass, 1);
    
    // Activate ability
    GASIntegration->ActivateAbility(UnitHandle, AbilityTag);
}
```

## Step 9: Implement Visual Representation

The plugin provides two ways to render units: Niagara-based vertex animation for large numbers of distant units, and skeletal meshes for close-up units.

### Niagara System

1. Get the Niagara System from the Mass Unit Subsystem
2. The system automatically updates unit visuals in the subsystem's tick function

### Mesh Pool

For units that need skeletal mesh representation:

1. Get the Mesh Pool from the Mass Unit Subsystem
2. Transition units to skeletal mesh when needed:

```
UUnitMeshPool* MeshPool = Subsystem->GetMeshPool();
if (MeshPool)
{
    // Transition to skeletal mesh
    MeshPool->TransitionToSkeletal(UnitHandle);
    
    // Get the skeletal mesh component
    USkeletalMeshComponent* MeshComponent = MeshPool->GetMeshForUnit(UnitHandle);
    
    // Transition back to vertex animation
    MeshPool->TransitionToVertex(UnitHandle);
}
```

## Step 10: Handle Unit Navigation

The Navigation System handles pathfinding for units.

### Blueprint Implementation

1. Get the Navigation System from the Mass Unit Subsystem
2. Request paths for units:

```
UMassUnitNavigationSystem* NavigationSystem = Subsystem->GetNavigationSystem();
if (NavigationSystem)
{
    // Request path
    NavigationSystem->RequestPath(UnitHandle, Destination);
}
```

### C++ Implementation

```cpp
UMassUnitNavigationSystem* NavigationSystem = Subsystem->GetNavigationSystem();
if (NavigationSystem)
{
    // Request path
    NavigationSystem->RequestPath(UnitHandle, Destination);
}
```

## Step 11: Implement Gameplay Events

The Gameplay Event System allows units to respond to events.

### Blueprint Implementation

1. Get the Gameplay Event System from the Mass Unit Subsystem
2. Dispatch events:

```
UUnitGameplayEventSystem* EventSystem = Subsystem->GetGameplayEventSystem();
if (EventSystem)
{
    // Dispatch event
    FGameplayEventData EventData;
    EventSystem->DispatchEvent(EventTag, UnitHandle, EventData);
}
```

### C++ Implementation

```cpp
UUnitGameplayEventSystem* EventSystem = Subsystem->GetGameplayEventSystem();
if (EventSystem)
{
    // Dispatch event
    FGameplayEventData EventData;
    EventSystem->DispatchEvent(EventTag, UnitHandle, EventData);
}
```

## Step 12: Optimize Performance

The plugin includes several optimization techniques for handling large numbers of units.

### LOD System

Units automatically transition between different levels of detail based on distance from the camera:

- Distant units: Niagara-based vertex animation
- Close units: Skeletal meshes with full animation

### Spatial Partitioning

The plugin uses spatial partitioning to efficiently process only relevant units:

- Only update units that are visible or near the player
- Batch process units for better performance

### Update Frequency

You can adjust the update frequency for different systems:

- Visual updates: Every frame for close units, less frequent for distant units
- Navigation updates: Batch process path requests
- Formation updates: Adjust based on distance and visibility

## Step 13: Debug and Troubleshoot

The plugin includes several debugging tools to help identify and fix issues.

### Common Issues

1. **Missing Dependencies**: Ensure all required plugins are enabled
2. **Navigation Issues**: Check navigation mesh generation and agent settings
3. **Visual Glitches**: Verify texture and mesh assets
4. **Performance Problems**: Adjust LOD settings and update frequencies

### Debugging Tools

1. **Visual Debugging**: Enable debug visualization for formations and paths
2. **Performance Monitoring**: Use Unreal's built-in profiling tools
3. **Logging**: Check the output log for plugin-specific messages

## Conclusion

By following this guide, you should now have the Mass Unit System plugin integrated into your project. The plugin provides a powerful foundation for creating games with thousands of units, with seamless integration with the Gameplay Ability System.

For more detailed information, refer to the plugin's technical documentation and API reference.
