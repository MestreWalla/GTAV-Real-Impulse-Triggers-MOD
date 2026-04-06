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

// Manual test state
static auto g_manualTestStartTime = std::chrono::steady_clock::now();
static int g_activeTestEvent = -1;

void ApplyMotor(const MotorConfig& m, std::chrono::steady_clock::time_point startTime, double& outVal, bool continuous = false) {
    if (!m.en) return;
    if (continuous) {
        outVal = std::max(outVal, (double)m.val);
    } else {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
        if (elapsed < m.dur) {
            outVal = std::max(outVal, (double)m.val);
        }
    }
}

void ApplyProfile(const VibeProfile& p, std::chrono::steady_clock::time_point startTime, double& lt, double& rt, double& lm, double& rm, bool continuous = false) {
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
            case 9: tp = &s.aiming; break; case 10: tp = &s.shootPistol; break; case 11: tp = &s.shootSMG; break;
            case 12: tp = &s.shootShotgun; break; case 13: tp = &s.shootHeavy; break; case 14: tp = &s.shootOther; break;
        }
        if (tp) {
            bool isCont = (g_activeTestEvent == 7);
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
    if (data.isAiming) {
        if (!g_wasAiming) {
            g_lastAimStartTime = now;
            Menu::Log("RECV: AIM_START", "~y~");
        }
        ApplyProfile(s.aiming, g_lastAimStartTime, ltV, rtV, lmV, rmV);
    }
    g_wasAiming = data.isAiming;

    // 3. Shooting Logic
    if (data.isShooting) {
        g_lastShotStartTime = now;
        Menu::Log("RECV: WEAPON_FIRE (GRP:" + std::to_string(data.weaponGroup) + ")", "~y~");
    }
    
    VibeProfile* activeShot = &s.shootOther;
    switch(data.weaponGroup) {
        case 860033945: activeShot = &s.shootShotgun; break;
        case 4166763944: activeShot = &s.shootPistol; break;
        case 970312471: activeShot = &s.shootSMG; break;
        case 3082541098: activeShot = &s.shootHeavy; break;
    }
    ApplyProfile(*activeShot, g_lastShotStartTime, ltV, rtV, lmV, rmV);

    // 4. Vehicle Logic
    if (data.isInVehicle) {
        if (data.gear != g_lastGear && data.gear > 0) {
            g_lastShiftStartTime = now;
            Menu::Log("RECV: GEAR_SHIFT", "~y~");
        }
        g_lastGear = data.gear;
        ApplyProfile(s.gearShift, g_lastShiftStartTime, ltV, rtV, lmV, rmV);

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