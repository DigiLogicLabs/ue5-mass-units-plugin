@echo off
REM Build script for Mass Unit System plugin

REM Set the path to your Unreal Engine installation
REM Set the path to your Unreal Engine installation
IF NOT "%UE_ENGINE_PATH%" == "" (
    set UE_PATH=%UE_ENGINE_PATH%
) ELSE (
    REM Attempt to find Unreal Engine installation if UE_ENGINE_PATH is not set
    REM This is a common default installation path, adjust as needed
    set UE_PATH=C:\Program Files\Epic Games\UE_5.6
    IF NOT EXIST "%UE_PATH%" (
        set UE_PATH=D:\Program Files\Epic Games\UE_5.6
    )
    IF NOT EXIST "%UE_PATH%" (
        echo Warning: UE_ENGINE_PATH environment variable not set and common installation paths not found.
        echo Please set UE_ENGINE_PATH or modify build_plugin.bat with your Unreal Engine 5.6 path.
        pause
        exit /b 1
    )
)

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
