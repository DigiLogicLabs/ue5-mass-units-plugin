# UE5 Mass Unit System Plugin

This plugin provides a high-performance, large-scale unit system for Unreal Engine 5.6, designed to enable the rendering and simulation of thousands of units simultaneously. It features seamless integration with the Gameplay Ability System (GAS) and supports advanced behaviors like dynamic pathfinding and formation management.

## Key Features

- **Mass Entity System Integration**: Efficient data-oriented management for large numbers of entities.
- **Niagara-Based Rendering**: GPU-accelerated rendering for thousands of units.
- **Vertex Animation System**: High-performance animation without skeletal overhead.
- **GAS Integration**: Seamless connection with existing GAS workflows.
- **Dynamic Pathfinding & Formation System**: Efficient navigation and sophisticated formation management.
- **LOD System**: Performance optimization through level of detail transitions.

## Getting Started

Follow these minimal steps to integrate the Mass Unit System plugin into your Unreal Engine 5.6 project.

### Prerequisites

- **Unreal Engine 5.6** or later installed.
- An existing Unreal Engine 5.6 project.
- Basic understanding of Unreal Engine Blueprint and C++.

### Installation

#### Option 1: Clone Directly into Your Project (Recommended)

1.  Navigate to your Unreal Engine project's root directory.
2.  Create a `Plugins` folder if it doesn't already exist.
3.  Open your terminal or command prompt in the `Plugins` folder.
4.  Clone the repository:
    ```bash
    git clone https://github.com/DigiLogicLabs/ue5-mass-units-plugin.git MassUnitSystem
    ```
5.  Right-click on your `.uproject` file and select "Generate Visual Studio project files" (or your IDE's equivalent).
6.  Open your project in Unreal Engine. The plugin should be automatically detected and compiled.

#### Option 2: Download and Copy

1.  Download the plugin repository as a ZIP file from GitHub.
2.  Extract the contents into your project's `Plugins` directory. Ensure the extracted folder is named `MassUnitSystem`.
3.  Right-click on your `.uproject` file and select "Generate Visual Studio project files".
4.  Open your project in Unreal Engine. The plugin should be automatically detected and compiled.

### Enable Required Dependencies

The Mass Unit System plugin relies on several other Unreal Engine plugins. Ensure these are enabled in your project:

1.  Open your project in Unreal Engine.
2.  Go to `Edit > Plugins`.
3.  Search for and enable the following plugins:
    -   **Mass Entity**
    -   **Mass Gameplay**
    -   **Niagara**
    -   **Gameplay Abilities**
    -   **GASCompanion** (optional but highly recommended for full GAS integration)
4.  Restart the Unreal Engine editor when prompted.

## Usage

For detailed information on how to use the plugin, including configuring project settings, initializing the subsystem, creating unit templates, spawning units, and integrating with GAS, please refer to the `integration_guide.md` and other documentation files in the `docs/` directory of this repository.

## Contributing

We welcome contributions! Please refer to the `CONTRIBUTING.md` (if available) for guidelines on how to contribute to this project.

## License

This project is licensed under the [Your License Here] - see the `LICENSE` file for details.

