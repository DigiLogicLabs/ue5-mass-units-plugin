#!/bin/bash

# UE5 Mass Unit System Plugin - Linux/macOS Build Script
# This script uses Unreal Automation Tool (UAT) to build the plugin.

# Default UE_PATH - Update this to your Unreal Engine installation path
# Example: /home/user/UnrealEngine/Engine
UE_PATH="/opt/unreal-engine/Engine"

# Check if UE_PATH exists
if [ ! -d "$UE_PATH" ]; then
    echo "Error: Unreal Engine path not found at $UE_PATH"
    echo "Please update the UE_PATH variable in this script."
    exit 1
fi

UAT_PATH="$UE_PATH/Build/BatchFiles/RunUAT.sh"
PLUGIN_PATH="$(pwd)/MassUnitSystem.uplugin"
OUTPUT_PATH="$(pwd)/Build/Plugin"

echo "Building Mass Unit System Plugin..."
echo "UE Path: $UE_PATH"
echo "Plugin: $PLUGIN_PATH"
echo "Output: $OUTPUT_PATH"

# Run UAT to build the plugin
"$UAT_PATH" BuildPlugin -Plugin="$PLUGIN_PATH" -Package="$OUTPUT_PATH" -Rocket

if [ $? -eq 0 ]; then
    echo "Build Successful!"
    echo "Built plugin located at: $OUTPUT_PATH"
else
    echo "Build Failed!"
    exit 1
fi
