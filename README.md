# GTA V Impulse Triggers Mod (C++) (ALPHA)

:warning: Do not use online to avoid being banned :warning:

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![Status](https://img.shields.io/badge/status-alpha-yellow)

![Project Banner](https://github.com/MestreWalla/GTAV-Real-Impulse-Triggers-MOD/blob/main/Impulse%20Triggers.png "Banner of My Project")

> Feel every detail of Los Santos in your hands. This plugin adds native support for **Impulse Triggers (Xbox)**, transforming GTA V’s physics into precise and immersive vibrations.

## 🎮 In-Game Configuration Menu (F5)

The mod is fully configurable in real time. Press **F5** to open the menu and adjust every detail. While the menu is open, frontend inputs are blocked so GTA’s phone and game controls do not interfere with menu navigation.

### What's New in the Current Version

- **38 Haptic Events**: Expanded from a small set to a fully integrated collection of tactile feedback events.
- **Combat Refinement**: `Shoot_Other`, `Aiming`, `Reloading`, `Weapon_Switch`, `Melee_Impact`, `Getting_Shot`, and `Explosion` are all supported.
- **Weapon Consolidation**: All weapon firing now uses the consolidated `Shoot_Other` profile.
- **Aiming Pulse**: `Aiming` now gives a brief vibration pulse when starting to aim, instead of continuous vibration while holding.
- **Event Overlap**: Multiple events can occur simultaneously, with the strongest vibration per motor taking precedence.
- **Code Cleanup**: Refactored with helper functions to reduce duplication and improve maintainability.
- **Stronger Feedback**: Right trigger shooting is now full-strength and left/right motors also pulse with consistent time.
- **Menu Input Fix**: Arrow keys no longer open the phone while the menu is active.
- **Detailed Diagnostics**: In-game logs show event start, completion, and release states.

## ✨ Features and Events

### 🚗 Vehicle Physics (8 events)

- **Gear Shift**
- **ABS**
- **Traction Loss**
- **Drifting**
- **Suspension Bump**
- **Collision**
- **Engine Stutter**
- **Engine RPM**

### 🔫 Combat & Weapons (11 events)

- **Shoot_Other** (all weapons consolidated)
- **Aiming**
- **Damage Taken**
- **Melee Impact**
- **Reloading**
- **Weapon Switch**
- **Getting Shot**
- **Explosion**
- **Blast Wave**
- **Electrocuted**
- **Burning**

### 🚗 Vehicle Damage & Types (5 events)

- **Tire Burst**
- **Vehicle On Fire**
- **Helicopter Blades**
- **Train Vibration**
- **Airplane Turbulence**

### 🚶 Player Movement (6 events)

- **Climbing**
- **Jumping**
- **Falling Impact**
- **Swimming**
- **Parachuting**
- **Sliding**

### ⚡ Environmental Events (3 events)

- **Lightning Strike**
- **Earthquake**
- **Police Chase**

### 👤 Player Status (3 events)

- **Stunned**
- **Drowning**
- **Poisoned**

### 🛠 Notes

- `Aiming` gives a brief pulse when starting to aim, not continuous while holding.
- `Shoot_Other` sends full RT power with strong LM/RM support.
- Multiple events can overlap, with the strongest vibration per motor winning.
- The menu blocks frontend controls so F5 navigation is isolated from gameplay.

## 📋 Hardware Requirements

To feel the **Impulse Triggers** (motors inside the triggers), Windows requires:  

1. **Controller**: Original Xbox One or Xbox Series.  
2. **Connection**: Must be connected via **USB Cable** or **Official Wireless Adapter**.  
   - *Note: Standard Bluetooth does not support independent trigger vibration.*  

## 🛠️ Installation

1. Make sure you have **Script Hook V** installed.  
2. Copy the file `GTAVImpulseTriggers.asi` to the GTA V root folder.  
3. Launch the game and use the **F5** menu to customize your experience.

## 🎛️ Motor Control

Each event can be customized with these parameters:

- **Enabled/Disabled** - Toggle the effect on/off
- **Power (0-100%)** - Intensity of the vibration
- **Duration (0-500ms)** - How long the vibration lasts
- **4 Motors**: Left Trigger (LT), Right Trigger (RT), Left Motor (LM), Right Motor (RM)

---

*Transforming data into sensations. Next-gen haptic experience on PC.* ✨
