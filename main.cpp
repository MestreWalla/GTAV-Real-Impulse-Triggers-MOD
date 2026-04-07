#define NOMINMAX
#include "inc/main.h"
#include "ControllerManager.h"
#include "Telemetry.h"
#include "Config.h"
#include "Menu.h"
#include <chrono>
#include <thread>
#include <memory>
#include <algorithm>
#include <fstream>

static std::unique_ptr<ControllerManager> g_controller;
static int g_lastGear = 0;
static int g_lastPlayerHealth = 200;
static bool g_isInitialized = false;
static auto g_initTime = std::chrono::steady_clock::now();

static auto g_lastShiftStartTime = std::chrono::steady_clock::now();
static auto g_lastDamageStartTime = std::chrono::steady_clock::now();
static auto g_lastShotStartTime = std::chrono::steady_clock::now();
static auto g_lastCollisionStartTime = std::chrono::steady_clock::now();
static auto g_lastAimStartTime = std::chrono::steady_clock::now();
static bool g_wasAiming = false;
static bool g_wasShooting = false;
static bool g_aimPulseDone = false;

// Combat events
static auto g_lastMeleeStartTime = std::chrono::steady_clock::now();
static auto g_lastReloadStartTime = std::chrono::steady_clock::now();
static auto g_lastWeaponSwitchStartTime = std::chrono::steady_clock::now();
static auto g_lastExplosionStartTime = std::chrono::steady_clock::now();
static bool g_wasMeleeing = false;
static bool g_wasReloading = false;
static bool g_wasWeaponSwitching = false;
static bool g_isMeleeingActive = false;
static bool g_isReloadingActive = false;
static bool g_isWeaponSwitchActive = false;
static bool g_hadExplosionNearby = false;

// Vehicle events
static auto g_lastTireBurstStartTime = std::chrono::steady_clock::now();
static auto g_lastVehicleExplosionStartTime = std::chrono::steady_clock::now();
static auto g_lastHelicopterStartTime = std::chrono::steady_clock::now();
static auto g_lastPlaneStartTime = std::chrono::steady_clock::now();
static auto g_lastTrainStartTime = std::chrono::steady_clock::now();
static bool g_hadTireBurst = false;
static bool g_wasVehicleOnFire = false;
static bool g_wasInHelicopter = false;
static bool g_wasInPlane = false;
static bool g_wasInTrain = false;

// Player state events
static auto g_lastJumpStartTime = std::chrono::steady_clock::now();
static auto g_lastFallingStartTime = std::chrono::steady_clock::now();
static auto g_lastClimbingStartTime = std::chrono::steady_clock::now();
static auto g_lastStunnedStartTime = std::chrono::steady_clock::now();
static auto g_lastPoliceCaseStartTime = std::chrono::steady_clock::now();
static bool g_wasJumping = false;
static bool g_wasFalling = false;
static bool g_wasClimbing = false;
static bool g_wasStunned = false;
static bool g_wasInPoliceChase = false;
static bool g_wasInVehicle = false;
static bool g_isShootingActive = false;
static bool g_isAimingActive = false;

// Manual test state
static auto g_manualTestStartTime = std::chrono::steady_clock::now();
static int g_activeTestEvent = -1;

// Apply a single motor config into the current output values.
// - m.en: whether this motor is enabled in the profile.
// - startTime: when the event began, used to enforce duration.
// - outVal: current accumulated strength for this motor.
// - continuous: if true, this event stays active continually and ignores duration.
void ApplyMotor(const MotorConfig& m, std::chrono::steady_clock::time_point startTime, double& outVal, bool continuous = false) {
    if (!m.en) return;
    if (continuous) {
        // Continuous events use fixed power while the state remains true.
        outVal = std::max(outVal, (double)m.val);
    } else {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
        if (elapsed < m.dur) {
            // Pulsed events apply only for the configured duration.
            outVal = std::max(outVal, (double)m.val);
        }
    }
}

// Forward declaration
void ApplyProfile(const VibeProfile& p, std::chrono::steady_clock::time_point startTime, double& lt, double& rt, double& lm, double& rm, bool continuous);

// Helper function to handle pulsed events (start on condition, apply for duration, then stop)
void HandlePulsedEvent(bool currentState, bool& previousState, bool& isActive, std::chrono::steady_clock::time_point& startTime, const VibeProfile& profile, double& ltV, double& rtV, double& lmV, double& rmV, const std::string& logStart, const std::string& logEnd) {
    if (currentState && !previousState) {
        isActive = true;
        startTime = std::chrono::steady_clock::now();
        Menu::Log(logStart, "~y~");
    }

    if (isActive) {
        int maxDur = std::max({profile.lt.dur, profile.rt.dur, profile.lm.dur, profile.rm.dur});
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
        if (elapsed >= maxDur) {
            isActive = false;
            Menu::Log(logEnd, "~w~");
        } else {
            ApplyProfile(profile, startTime, ltV, rtV, lmV, rmV, false);
        }
    }

    if (!currentState && previousState) {
        Menu::Log(logEnd + "_RELEASE", "~o~");
    }
    previousState = currentState;
}

// Apply an entire VibeProfile to all four motors.
// This is the core place where config values are turned into actual motor outputs.
void ApplyProfile(const VibeProfile& p, std::chrono::steady_clock::time_point startTime, double& lt, double& rt, double& lm, double& rm, bool continuous = false) {
    if (!p.enabled) return;
    ApplyMotor(p.lt, startTime, lt, continuous);
    ApplyMotor(p.rt, startTime, rt, continuous);
    ApplyMotor(p.lm, startTime, lm, continuous);
    ApplyMotor(p.rm, startTime, rm, continuous);
}

void UpdateScript() {
    bool connected = g_controller && g_controller->IsConnected();
    
    static bool wasMenuOpen = false;
    if (Menu::IsOpen() != wasMenuOpen) {
        if (Menu::IsOpen()) Config::Reload();
        wasMenuOpen = Menu::IsOpen();
    }
    
    Menu::Update(connected);
    VehicleTelemetry data = Telemetry::GetData();
    Settings& s = Config::Get();

    auto now = std::chrono::steady_clock::now();

    if (!g_isInitialized) {
        if (CAM::IS_SCREEN_FADED_IN()) {
            g_lastGear = data.gear; g_lastPlayerHealth = data.playerHealth;
            g_initTime = now; g_isInitialized = true;
        }
        return; 
    }

    if (std::chrono::duration_cast<std::chrono::seconds>(now - g_initTime).count() < 2) {
        g_lastGear = data.gear; g_lastPlayerHealth = data.playerHealth; return;
    }

    if (s.showDebugHUD) Menu::DrawHUD(data);

    double rtV = 0.0, ltV = 0.0, rmV = 0.0, lmV = 0.0;

    // 0. Manual Test Trigger
    int testIdx = Menu::GetManualTestEvent();
    if (testIdx != -1) {
        g_activeTestEvent = testIdx;
        g_manualTestStartTime = now;
        Menu::ClearManualTest();
    }

    if (g_activeTestEvent != -1) {
        VibeProfile* tp = nullptr;
        switch(g_activeTestEvent) {
            case 0: tp = &s.gearShift; break; case 1: tp = &s.abs; break; case 2: tp = &s.tractionLoss; break;
            case 3: tp = &s.drifting; break; case 4: tp = &s.suspensionBump; break; case 5: tp = &s.collision; break;
            case 6: tp = &s.engineStutter; break; case 7: tp = &s.engineRPM; break; case 8: tp = &s.damageTaken; break;
            case 9: tp = &s.aiming; break; case 10: tp = &s.shootOther; break;
            case 11: tp = &s.meleeImpact; break; case 12: tp = &s.reloading; break; case 13: tp = &s.weaponSwitch; break;
            case 14: tp = &s.gettingShot; break; case 15: tp = &s.explosion; break; case 16: tp = &s.blastWave; break;
            case 17: tp = &s.electrocuted; break; case 18: tp = &s.burning; break; case 19: tp = &s.tireBurst; break;
            case 20: tp = &s.vehicleOnFire; break; case 21: tp = &s.helicopterBlades; break; case 22: tp = &s.trainVibration; break;
            case 23: tp = &s.airplaneTurbulence; break; case 24: tp = &s.climbing; break; case 25: tp = &s.jumping; break;
            case 26: tp = &s.fallingImpact; break; case 27: tp = &s.swimming; break; case 28: tp = &s.parachuting; break;
            case 29: tp = &s.sliding; break; case 30: tp = &s.lightningStrike; break; case 31: tp = &s.earthquake; break;
            case 32: tp = &s.policeChase; break; case 33: tp = &s.stunned; break; case 34: tp = &s.drowning; break;
            case 35: tp = &s.poisoned; break; case 36: tp = &s.windowBreak; break; case 37: tp = &s.vehicleExplosion; break;
        }
        if (tp) {
            bool isCont = (g_activeTestEvent == 7 || g_activeTestEvent == 9 || g_activeTestEvent == 20 || g_activeTestEvent == 21 || g_activeTestEvent == 22 || g_activeTestEvent == 23 || g_activeTestEvent == 24 || g_activeTestEvent == 27 || g_activeTestEvent == 28);
            ApplyProfile(*tp, g_manualTestStartTime, ltV, rtV, lmV, rmV, isCont);
            if (!isCont && std::chrono::duration_cast<std::chrono::milliseconds>(now - g_manualTestStartTime).count() > 1000) {
                g_activeTestEvent = -1;
            }
        }
    }

    // 1. Damage
    if (data.playerHealth < g_lastPlayerHealth) {
        g_lastDamageStartTime = now;
        Menu::Log("RECV: DAMAGE_TAKEN (" + std::to_string(data.playerHealth) + ")", "~y~");
    }
    g_lastPlayerHealth = data.playerHealth;
    ApplyProfile(s.damageTaken, g_lastDamageStartTime, ltV, rtV, lmV, rmV);

    // 2. Aiming Logic
    // Aiming gives a brief pulse when starting to aim, not continuous while holding.
    // The pulse lasts for the maximum motor duration in the aiming profile.
    int aimMaxDur = std::max({s.aiming.lt.dur, s.aiming.rt.dur, s.aiming.lm.dur, s.aiming.rm.dur});
    if (data.isAiming && !g_wasAiming) {
        // Just started aiming: trigger the pulse
        g_isAimingActive = true;
        g_lastAimStartTime = now;
        g_aimPulseDone = false;
        Menu::Log("RECV: AIM_START", "~y~");
    }

    if (g_isAimingActive && !g_aimPulseDone) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_lastAimStartTime).count();
        if (elapsed >= aimMaxDur) {
            g_isAimingActive = false;
            g_aimPulseDone = true;
            Menu::Log("AIM_PULSE_END", "~w~");
        } else {
            ApplyProfile(s.aiming, g_lastAimStartTime, ltV, rtV, lmV, rmV);
        }
    }
    if (!data.isAiming && g_wasAiming) {
        // Stopped aiming: reset for next time
        g_aimPulseDone = false;
        Menu::Log("AIM_RELEASE", "~o~");
    }
    g_wasAiming = data.isAiming;

    // 3. Shooting Logic
    HandlePulsedEvent(data.isShooting, g_wasShooting, g_isShootingActive, g_lastShotStartTime, s.shootOther, ltV, rtV, lmV, rmV, "RECV: WEAPON_FIRE (GRP:" + std::to_string(data.weaponGroup) + ")", "SHOT_END");

    // 4. Melee Combat
    HandlePulsedEvent(data.isMeleeing, g_wasMeleeing, g_isMeleeingActive, g_lastMeleeStartTime, s.meleeImpact, ltV, rtV, lmV, rmV, "RECV: MELEE_IMPACT", "MELEE_END");

    // 5. Reloading
    HandlePulsedEvent(data.isReloading, g_wasReloading, g_isReloadingActive, g_lastReloadStartTime, s.reloading, ltV, rtV, lmV, rmV, "RECV: RELOADING", "RELOAD_END");

    // 6. Weapon Switch
    HandlePulsedEvent(data.isWeaponSwitching, g_wasWeaponSwitching, g_isWeaponSwitchActive, g_lastWeaponSwitchStartTime, s.weaponSwitch, ltV, rtV, lmV, rmV, "RECV: WEAPON_SWITCH", "WEAPON_SWITCH_END");

    // 7. Explosions Nearby
    if (data.hasExplosionNearby) {
        if (!g_hadExplosionNearby) {
            g_lastExplosionStartTime = now;
            Menu::Log("RECV: EXPLOSION (Distance: " + std::to_string((int)data.explosionDistance) + ")", "~y~");
        }
        ApplyProfile(s.explosion, g_lastExplosionStartTime, ltV, rtV, lmV, rmV);
    }
    g_hadExplosionNearby = data.hasExplosionNearby;

    // 7. Player State Events
    if (data.isJumping) {
        if (!g_wasJumping) {
            g_lastJumpStartTime = now;
            Menu::Log("RECV: JUMPING", "~y~");
        }
        ApplyProfile(s.jumping, g_lastJumpStartTime, ltV, rtV, lmV, rmV);
    }
    g_wasJumping = data.isJumping;

    if (data.isFalling) {
        if (!g_wasFalling) {
            g_lastFallingStartTime = now;
            Menu::Log("RECV: FALLING (Height: " + std::to_string((int)data.fallHeight) + "m)", "~y~");
        }
        ApplyProfile(s.fallingImpact, g_lastFallingStartTime, ltV, rtV, lmV, rmV);
    }
    g_wasFalling = data.isFalling;

    if (data.isClimbing) {
        if (!g_wasClimbing) {
            g_lastClimbingStartTime = now;
            Menu::Log("RECV: CLIMBING", "~y~");
        }
        ApplyProfile(s.climbing, g_lastClimbingStartTime, ltV, rtV, lmV, rmV, true);
    }
    g_wasClimbing = data.isClimbing;

    if (data.isStunned) {
        if (!g_wasStunned) {
            g_lastStunnedStartTime = now;
            Menu::Log("RECV: STUNNED", "~y~");
        }
        ApplyProfile(s.stunned, g_lastStunnedStartTime, ltV, rtV, lmV, rmV, true);
    }
    g_wasStunned = data.isStunned;

    if (data.isSwimming && !g_wasInVehicle) {
        double swimIntensity = 0.5;
        ltV = std::max(ltV, (double)(s.swimming.lt.val * swimIntensity));
        rtV = std::max(rtV, (double)(s.swimming.rt.val * swimIntensity));
        lmV = std::max(lmV, (double)(s.swimming.lm.val * swimIntensity));
        rmV = std::max(rmV, (double)(s.swimming.rm.val * swimIntensity));
    }

    if (data.isParachuting) {
        ltV = std::max(ltV, (double)s.parachuting.lt.val);
        rtV = std::max(rtV, (double)s.parachuting.rt.val);
        lmV = std::max(lmV, (double)s.parachuting.lm.val);
        rmV = std::max(rmV, (double)s.parachuting.rm.val);
    }

    // Police Chase - Temporarily disabled due to excessive vibration
    /*
    if (data.inPoliceChase) {
        if (!g_wasInPoliceChase) {
            g_lastPoliceCaseStartTime = now;
            Menu::Log("RECV: POLICE_CHASE", "~y~");
        }
        ApplyProfile(s.policeChase, g_lastPoliceCaseStartTime, ltV, rtV, lmV, rmV, true);
    }
    g_wasInPoliceChase = data.inPoliceChase;
    */

    // Vehicle was needed for tracking
    static bool g_wasInVehicle = data.isInVehicle;

    // 8. Vehicle Logic
    if (data.isInVehicle) {
        if (data.gear != g_lastGear && data.gear > 0) {
            g_lastShiftStartTime = now;
            Menu::Log("RECV: GEAR_SHIFT", "~y~");
        }
        g_lastGear = data.gear;
        ApplyProfile(s.gearShift, g_lastShiftStartTime, ltV, rtV, lmV, rmV);

        // Vehicle state detection
        if (data.hasBlowoutTires) {
            if (!g_hadTireBurst) {
                g_lastTireBurstStartTime = now;
                Menu::Log("RECV: TIRE_BURST", "~y~");
            }
            ApplyProfile(s.tireBurst, g_lastTireBurstStartTime, ltV, rtV, lmV, rmV, true);
        }
        g_hadTireBurst = data.hasBlowoutTires;

        if (data.isVehicleOnFire) {
            if (!g_wasVehicleOnFire) {
                g_lastVehicleExplosionStartTime = now;
                Menu::Log("RECV: VEHICLE_ON_FIRE", "~y~");
            }
            ApplyProfile(s.vehicleOnFire, g_lastVehicleExplosionStartTime, ltV, rtV, lmV, rmV, true);
        }
        g_wasVehicleOnFire = data.isVehicleOnFire;

        if (data.isInHelicopter) {
            if (!g_wasInHelicopter) {
                g_lastHelicopterStartTime = now;
                Menu::Log("RECV: HELICOPTER_BLADES", "~y~");
            }
            ApplyProfile(s.helicopterBlades, g_lastHelicopterStartTime, ltV, rtV, lmV, rmV, true);
        }
        g_wasInHelicopter = data.isInHelicopter;

        if (data.isInPlane) {
            if (!g_wasInPlane) {
                g_lastPlaneStartTime = now;
                Menu::Log("RECV: AIRPLANE_TURBULENCE", "~y~");
            }
            ApplyProfile(s.airplaneTurbulence, g_lastPlaneStartTime, ltV, rtV, lmV, rmV, true);
        }
        g_wasInPlane = data.isInPlane;

        if (data.isInTrain) {
            if (!g_wasInTrain) {
                g_lastTrainStartTime = now;
                Menu::Log("RECV: TRAIN_VIBRATION", "~y~");
            }
            ApplyProfile(s.trainVibration, g_lastTrainStartTime, ltV, rtV, lmV, rmV, true);
        }
        g_wasInTrain = data.isInTrain;

        if (data.engineHealth < 400.0f && data.throttle > 0.1f) {
            static int stutterCounter = 0;
            if (++stutterCounter % 5 == 0) ApplyProfile(s.engineStutter, now, ltV, rtV, lmV, rmV, true);
        }

        if (data.throttle > 0.1f && data.wheelSpeed > data.vehicleSpeed + 1.0f) {
            static bool osc = false; osc = !osc;
            if (osc) ApplyProfile(s.tractionLoss, now, ltV, rtV, lmV, rmV, true);
        }
        
        if (std::abs(data.lateralSpeed) > 3.0f && data.vehicleSpeed > 5.0f && !data.isAirborne) {
            double mult = std::min(1.0, std::abs(data.lateralSpeed) / 12.0);
            ltV = std::max(ltV, (double)(s.drifting.lt.val * mult));
            rtV = std::max(rtV, (double)(s.drifting.rt.val * mult));
            lmV = std::max(lmV, (double)(s.drifting.lm.val * mult));
            rmV = std::max(rmV, (double)(s.drifting.rm.val * mult));
        }

        if (data.brake > 0.1f && data.wheelSpeed < data.vehicleSpeed - 0.5f) {
            static bool oscLT = false; oscLT = !oscLT;
            if (oscLT) ApplyProfile(s.abs, now, ltV, rtV, lmV, rmV, true);
        }

        if (!data.isAirborne && std::abs(data.zVelocity) > 1.5f && data.vehicleSpeed > 5.0f) {
            double bMult = std::min(1.0, std::abs(data.zVelocity) / 5.0);
            lmV = std::max(lmV, (double)(s.suspensionBump.lm.val * bMult));
            rmV = std::max(rmV, (double)(s.suspensionBump.rm.val * bMult));
        }

        if (data.collisionImpact > 5.0f) {
            g_lastCollisionStartTime = now;
            Menu::Log("RECV: COLLISION", "~y~");
        }
        ApplyProfile(s.collision, g_lastCollisionStartTime, ltV, rtV, lmV, rmV);

        if (data.rpm > 0.2f && !data.isAirborne) {
            rmV = std::max(rmV, (double)(s.engineRPM.rm.val * data.rpm));
            lmV = std::max(lmV, (double)(s.engineRPM.lm.val * data.rpm));
        }
    }

    if (g_controller) {
        g_controller->SetVibration(ltV, rtV, lmV, rmV);
        
        static bool lastVibeState = false;
        bool currentlyVibrating = (ltV > 0 || rtV > 0 || lmV > 0 || rmV > 0);
        
        if (currentlyVibrating && !lastVibeState) {
            char buf[64];
            sprintf_s(buf, "SEND: LT%.0f RT%.0f L%.0f R%.0f", ltV*100, rtV*100, lmV*100, rmV*100);
            Menu::Log(buf, "~g~");
        }
        lastVibeState = currentlyVibrating;
    }
}

void scriptMain() {
    try { winrt::init_apartment(winrt::apartment_type::multi_threaded); } catch (...) {}
    Config::Load();
    try { g_controller = std::make_unique<ControllerManager>(); } catch (...) {}
    while (true) {
        try { UpdateScript(); } catch (...) {}
        WAIT(0);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: scriptRegister(hModule, scriptMain); break;
    case DLL_PROCESS_DETACH: g_controller.reset(); break;
    }
    return TRUE;
}