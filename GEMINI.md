# GTA V Impulse Triggers Mod - Project Status

## Overview

This project is a high-performance C++ .asi plugin for GTA V (PC). It uses the `Windows.Gaming.Input` (WinRT) API to provide immersive haptic feedback through Xbox Impulse Triggers (LT/RT) and the main rumble motors, based on real-time game telemetry.

## Current Status

- **38 integrated haptic events** covering vehicle physics, combat, movement, damage, and environmental effects.
- **Menu input isolation** implemented so the F5 config UI does not open GTA’s phone or interfere with gameplay controls.
- **Code refactored** with helper functions for pulsed events, reducing duplication and improving maintainability.
- **Improved diagnostics** with event start/end logs and release signals for better tuning.

## Major Features

- **Combat Feedback**: consolidated weapon fire into `Shoot_Other`, plus aiming, reloading, weapon switching, melee, getting shot, and explosion feedback.
- **Aiming Pulse**: aiming now provides a brief vibration pulse when starting to aim, preventing continuous vibration while holding.
- **Event Overlap**: multiple haptic events can occur simultaneously, with the strongest motor values taking precedence.
- **Vehicle Coverage**: gear shifts, ABS, traction loss, drifting, suspension bumps, collisions, engine stutter, RPM, tire bursts, vehicle fire, and vehicle explosion.
- **Vehicle Type States**: helicopter, airplane, and train vibration effects.
- **Player Movement**: climbing, jumping, falling impact, swimming, parachuting, and sliding.
- **Environmental/Status Effects**: lightning, earthquake, police chase, stunned, drowning, and poisoned.

## Telemetry Improvements

- Added stable state detection for `Aiming`, `Shooting`, `Melee`, `Reloading`, and `Weapon_Switch`.
- Added weapon switch detection by comparing current weapon hashes across frames.
- Added minimum vibration durations so short state flickers do not cut off haptic output.
- Added menu-side frontend control blocking so arrow navigation stays inside the UI.

## Fixed Issues

- **Aiming pulse fixed**: aim vibration now gives a brief pulse when starting to aim, instead of continuous vibration while holding.
- **Shooting reset bug fixed**: shot vibration now persists for the configured duration instead of restarting or cutting out each frame.
- **Event overlap support**: multiple events can now coexist, with stronger vibrations overriding weaker ones per motor.
- **Phone menu bug fixed**: opening the menu no longer triggers GTA phone UI with directional keys.
- **Stronger shooting output**: right trigger now reaches 100% power, while main rumble motors provide robust support.

## Event Set

The mod currently supports the following events:

- GearShift, ABS, TractionLoss, Drifting, SuspensionBump, Collision, EngineStutter, EngineRPM
- DamageTaken, Aiming, Shoot_Other, Melee_Impact, Reloading, Weapon_Switch, Getting_Shot, Explosion, Blast_Wave, Electrocuted, Burning
- Tire_Burst, Vehicle_On_Fire, Helicopter_Blades, Train_Vibration, Airplane_Turbulence
- Climbing, Jumping, Falling_Impact, Swimming, Parachuting, Sliding
- Lightning_Strike, Earthquake, Police_Chase
- Stunned, Drowning, Poisoned
- Window_Break, Vehicle_Explosion

## Implementation Notes

- All core event detection uses GTA natives and lightweight state tracking.
- The menu and config system support live adjustments and profile saving.
- The plugin compiles to `compilado/bin/GTAVImpulseTriggers.asi` using MSVC / Visual Studio 2022.

## Build Requirements

- **Target:** `compilado/bin/GTAVImpulseTriggers.asi`
- **Libraries:** `WindowsApp.lib`, `Shlwapi.lib`, `User32.lib`, `compilado/lib/ScriptHookV.lib`
