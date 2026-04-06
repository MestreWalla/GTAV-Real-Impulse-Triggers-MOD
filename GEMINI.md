# GTA V Impulse Triggers Mod - Project Status

## Overview
This project is a high-performance C++ .asi plugin for GTA V (PC). It utilizes the `Windows.Gaming.Input` (WinRT) API to provide immersive haptic feedback via Xbox Impulse Triggers (LT/RT) and main rumble motors, based on real-time game physics and telemetry.

## Recent Changes (Advanced Diagnostics & Combat Refinement)
- **Granular Signal Logging:** 
  - **RECV (Yellow)**: Now logs specific game data like `Weapon Group IDs` and `Player Health` to verify exact game states.
  - **SEND (Green)**: Now displays real-time motor output values (e.g., `SEND: LT0 RT80 L20 R10`) allowing users to confirm exactly what intensities are being transmitted to the controller hardware.
- **Combat Logic Overhaul:**
  - **Weapon Detection**: Aiming and shooting haptics are now automatically disabled when the player is unarmed (prevents "ghost" vibrations when pressing LT/RT on foot).
  - **Single-Pulse Aiming**: Aiming now triggers a distinct one-time pulse (customizable duration) instead of a continuous vibration, improving immersion and reducing battery drain.
  - **Signal Priority**: Shooting signals now have top priority in the motor pipeline to ensure they aren't masked by background engine RPM vibration.
- **Menu System Fixes:** 
  - Resolved an index offset bug where menu selection would highlight the correct item but modify the parameter below it.
  - Separated event headers from interactive options for cleaner navigation.

## Implementation Details
- **Language:** C++17
- **SDK:** Script Hook V SDK
- **Input API:** Windows.Gaming.Input (WinRT)

### Core Modules
1. **ControllerManager:** High-level WinRT interface for thread-safe vibration control.
2. **Telemetry:** Reads physical vectors, collision impacts, player/vehicle health, and refined weapon detection.
3. **Menu System:** Custom native-based UI with multi-level state management.
4. **Config:** Advanced INI management with hot-reloading.

## Build Requirements
- **Target:** `compilado/bin/GTAVImpulseTriggers.asi` (MSVC VS 2022)
- **Libraries:** `WindowsApp.lib`, `Shlwapi.lib`, `User32.lib`, `compilado/lib/ScriptHookV.lib`
