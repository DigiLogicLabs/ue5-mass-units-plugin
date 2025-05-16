@echo off
REM Build script for Mass Unit System plugin

REM Set the path to your Unreal Engine installation
set UE_PATH=D:\Unreal Engine\UE_5.5

REM Set the plugin path
set PLUGIN_PATH=%~dp0

REM Set the build configuration
set BUILD_CONFIG=Development

REM Set the target platform
set TARGET_PLATFORM=Win64

REM Build the plugin
echo Building plugin at %PLUGIN_PATH% for %TARGET_PLATFORM% in %BUILD_CONFIG% configuration...
"%UE_PATH%\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="%PLUGIN_PATH%\MassUnitSystem.uplugin" -Package="%PLUGIN_PATH%\Binaries" -TargetPlatforms=%TARGET_PLATFORM% -Configuration=%BUILD_CONFIG%

REM Check if build was successful
if %ERRORLEVEL% NEQ 0 (
    echo Build failed with error code %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
) else (
    echo Build completed successfully!
    pause
)
