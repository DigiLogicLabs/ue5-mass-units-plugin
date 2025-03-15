# Mass Unit System Plugin: Sample Project Implementation Guide

## Introduction

This guide provides step-by-step instructions for implementing the Mass Unit System plugin in a sample Unreal Engine 5.5 project. By following these steps, you'll be able to create a demonstration of large-scale unit simulation using the plugin's features.

## Prerequisites

Before starting, ensure you have:

- Unreal Engine 5.5 or later installed
- A new or existing UE5 project
- The Mass Unit System plugin installed in your project
- GASCompanion plugin installed and enabled
- Basic knowledge of Unreal Engine Blueprint and C++ (for advanced customization)

## Step 1: Project Setup

1. **Create or Open a Project**:
   - Create a new project using the Third Person template, or open your existing project
   - Ensure the project is using Unreal Engine 5.5 or later

2. **Enable Required Plugins**:
   - Open the Plugins window (Edit > Plugins)
   - Enable the following plugins:
     - Mass Unit System
     - Mass Entity
     - Mass Gameplay
     - Niagara
     - Gameplay Abilities
     - GASCompanion

3. **Restart the Editor**:
   - After enabling the plugins, restart the Unreal Editor to apply changes

## Step 2: Configure Project Settings

1. **Set Up Navigation**:
   - Open Project Settings > Navigation Mesh
   - Set appropriate values for:
     - Cell Size: 10.0
     - Cell Height: 10.0
     - Agent Radius: 35.0
     - Agent Height: 200.0
   - Enable "Generate Navigation Only Around Navigation Invokers" for better performance

2. **Configure Collision Settings**:
   - Open Project Settings > Engine > Collision
   - Create a new collision profile named "MassUnit" with:
     - Collision Enabled: Query Only
     - Object Type: Pawn
     - Custom responses for appropriate channels

3. **Set Up Input Mappings** (for controlling units):
   - Open Project Settings > Engine > Input
   - Add the following action mappings:
     - "SelectUnits": Left Mouse Button
     - "CommandUnits": Right Mouse Button
     - "FormationCycle": F key

## Step 3: Create Basic Level

1. **Create a New Level**:
   - Create a new level or use an existing one
   - Add a floor plane with at least 10000x10000 units size
   - Add some obstacles and terrain features

2. **Add Navigation Volume**:
   - Add a Nav Mesh Bounds Volume that covers your playable area
   - Build navigation (Build > Build Navigation)

3. **Add Player Start**:
   - Place a Player Start actor in your level
   - Position it at an appropriate location for viewing the units

## Step 4: Create Unit Templates

1. **Create Unit Template Assets**:
   - In the Content Browser, right-click and select Create > Mass Unit System > Unit Template
   - Create at least three different unit templates:
     - "Infantry_Template"
     - "Archer_Template"
     - "Cavalry_Template"

2. **Configure Unit Templates**:
   - Open each template and configure:
     - Unit Type: Set appropriate gameplay tag (e.g., "Unit.Infantry")
     - Base Stats: Set health, damage, move speed
     - Visual: Assign skeletal mesh, static mesh, and textures
     - Team: Set team ID and color
     - Formation: Set default formation type

3. **Set Up Animations**:
   - Create or import animation assets for each unit type
   - Configure animation tags in the unit templates

## Step 5: Create Unit Spawner

1. **Create Spawner Blueprint**:
   - Create a new Blueprint class based on Actor
   - Name it "BP_MassUnitSpawner"

2. **Add Spawner Logic**:
   - Open the Blueprint and add the following components:
     - Scene Component (Root)
     - Box Component (for spawn area)

3. **Implement Spawn Functionality**:
   - In the Event Graph, create a "SpawnUnits" function with parameters:
     - UnitTemplate: UnitTemplate Object Reference
     - Count: Integer
     - TeamID: Integer
   - Implement the function to:
     - Get the Mass Unit Subsystem
     - Loop through Count
     - Generate random positions within the Box Component
     - Call UnitManager->CreateUnitFromTemplate for each position

4. **Add UI Controls** (optional):
   - Create a simple UI for controlling spawn parameters
   - Add buttons for spawning different unit types

## Step 6: Create Formation Controller

1. **Create Formation Controller Blueprint**:
   - Create a new Blueprint class based on Actor
   - Name it "BP_FormationController"

2. **Add Formation Management Logic**:
   - Add variables for:
     - SelectedUnits: Array of Mass Entity Handles
     - CurrentFormation: Name (String)
     - FormationSystem: Object Reference

3. **Implement Selection Logic**:
   - Create functions for:
     - SelectUnits: Perform a trace and select units
     - ClearSelection: Clear the current selection
     - CycleFormation: Change between formation types

4. **Implement Command Logic**:
   - Create a "CommandUnits" function that:
     - Performs a trace to find target location
     - Gets the Formation System from the Mass Unit Subsystem
     - Creates or updates a formation
     - Adds selected units to the formation
     - Sets the formation target to the trace hit location

## Step 7: Create Player Controller

1. **Create Custom Player Controller**:
   - Create a new Blueprint class based on Player Controller
   - Name it "BP_MassUnitPlayerController"

2. **Add References**:
   - Add variables for:
     - FormationController: Object Reference
     - UnitSpawner: Object Reference

3. **Implement Input Handling**:
   - In the Event Graph, implement input events:
     - SelectUnits: Call FormationController->SelectUnits
     - CommandUnits: Call FormationController->CommandUnits
     - FormationCycle: Call FormationController->CycleFormation

4. **Set Up Camera**:
   - Configure the camera for a top-down or RTS-style view
   - Implement camera movement with WASD and zoom with mouse wheel

## Step 8: Set Up Game Mode

1. **Create Custom Game Mode**:
   - Create a new Blueprint class based on Game Mode
   - Name it "BP_MassUnitGameMode"

2. **Configure Game Mode**:
   - Set Default Pawn Class to a suitable pawn
   - Set Player Controller Class to BP_MassUnitPlayerController

3. **Implement Initialization**:
   - In Begin Play:
     - Spawn the Formation Controller
     - Spawn the Unit Spawner
     - Set references in the Player Controller

4. **Set as Default Game Mode**:
   - In Project Settings > Maps & Modes, set the Default Game Mode to BP_MassUnitGameMode

## Step 9: Create HUD and UI

1. **Create HUD Blueprint**:
   - Create a new Blueprint class based on HUD
   - Name it "BP_MassUnitHUD"

2. **Create Widget Blueprint**:
   - Create a new Widget Blueprint
   - Name it "WBP_UnitControls"

3. **Design UI**:
   - Add buttons for:
     - Spawn Infantry (100)
     - Spawn Archers (100)
     - Spawn Cavalry (100)
     - Clear All Units

4. **Implement UI Logic**:
   - Connect buttons to functions in the Unit Spawner
   - Add unit count display
   - Add formation type display

## Step 10: Test and Optimize

1. **Initial Testing**:
   - Play the level and test basic functionality:
     - Unit spawning
     - Selection
     - Formation movement
     - Formation cycling

2. **Performance Testing**:
   - Gradually increase unit counts to test performance
   - Monitor frame rate and memory usage
   - Adjust LOD settings as needed

3. **Debug Visualization**:
   - Enable debug visualization for:
     - Formations
     - Unit states
     - Navigation paths

4. **Optimize Settings**:
   - Adjust the following settings for optimal performance:
     - LOD distances
     - Update frequencies
     - Culling distances

## Step 11: Add GAS Integration

1. **Create Ability Sets**:
   - Create GASCompanion Ability Sets for each unit type
   - Add appropriate abilities (Attack, Defend, Special)

2. **Create Attribute Sets**:
   - Create GASCompanion Attribute Sets for units
   - Define attributes like Health, Damage, Speed

3. **Configure GAS Integration**:
   - In each Unit Template, set up:
     - Default abilities
     - Base attributes
     - Effect tags

4. **Test Abilities**:
   - Create a test scenario where units use abilities
   - Verify that GAS integration works correctly

## Step 12: Add Advanced Features

1. **Implement Unit Transitions**:
   - Create logic to transition units between vertex animation and skeletal meshes
   - Test transitions when units are close to the camera

2. **Add Combat System**:
   - Implement basic combat between opposing teams
   - Add visual effects for attacks and damage

3. **Add Environment Interaction**:
   - Implement obstacle avoidance
   - Add terrain effects on movement speed

4. **Add Sound Effects**:
   - Add ambient unit sounds
   - Add formation movement sounds
   - Add combat sounds

## Step 13: Package and Share

1. **Clean Up Project**:
   - Remove any debug code
   - Organize content into appropriate folders
   - Document custom Blueprint functions

2. **Create Documentation**:
   - Document the sample project setup
   - Create a quick start guide
   - Add comments to key Blueprint nodes

3. **Package the Project**:
   - Package the project for your target platform
   - Test the packaged build

## Conclusion

By following these steps, you've created a sample project that demonstrates the capabilities of the Mass Unit System plugin. This project can serve as a foundation for your own game development or as a learning resource for understanding large-scale unit simulation in Unreal Engine 5.5.

For more advanced usage, refer to the plugin's technical documentation and API reference.
