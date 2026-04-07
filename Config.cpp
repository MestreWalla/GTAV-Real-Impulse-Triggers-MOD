#include "Config.h"
#include <windows.h>
#include <shlwapi.h>
#include <vector>
#include <tuple>

#pragma comment(lib, "Shlwapi.lib")

struct ProfileDefaults {
    bool enabled;
    bool en;
    int dur;
    float lt, rt, lm, rm;
};

Settings Config::m_settings;

std::wstring Config::GetConfigPath() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    PathRemoveFileSpecW(buffer);
    std::wstring path = buffer;
    path += L"\\GTAVImpulseTriggers.ini";
    return path;
}

// Load a single motor's config for a profile from the INI.
// This is the core place where LT/RT/LM/RM values are read from disk.
// Defaults are provided by the caller and are only used if the INI section/key is missing.
void Config::LoadMotor(const std::wstring& path, const std::wstring& section, const std::wstring& prefix, MotorConfig& m, bool dEn, int dDur, float dVal) {
    m.en = GetPrivateProfileIntW(section.c_str(), (prefix + L"_En").c_str(), dEn ? 1 : 0, path.c_str()) == 1;
    m.dur = GetPrivateProfileIntW(section.c_str(), (prefix + L"_Dur").c_str(), dDur, path.c_str());
    m.val = (float)GetPrivateProfileIntW(section.c_str(), (prefix + L"_Val").c_str(), (int)(dVal * 100), path.c_str()) / 100.0f;
}

// Save a single motor's config back to the INI.
void Config::SaveMotor(const std::wstring& path, const std::wstring& section, const std::wstring& prefix, const MotorConfig& m) {
    WritePrivateProfileStringW(section.c_str(), (prefix + L"_En").c_str(), m.en ? L"1" : L"0", path.c_str());
    WritePrivateProfileStringW(section.c_str(), (prefix + L"_Dur").c_str(), std::to_wstring(m.dur).c_str(), path.c_str());
    WritePrivateProfileStringW(section.c_str(), (prefix + L"_Val").c_str(), std::to_wstring((int)(m.val * 100)).c_str(), path.c_str());
}

// Load a full vibration profile from the INI.
// Each profile contains 4 motors: LT, RT, LM, RM.
// Default values are used only when the INI has no saved section/values.
void Config::LoadProfile(const std::wstring& path, const std::wstring& section, VibeProfile& p, bool dEnabled, bool dEn, int dDur, float dLt, float dRt, float dLm, float dRm) {
    p.enabled = GetPrivateProfileIntW(section.c_str(), L"Enabled", dEnabled ? 1 : 0, path.c_str()) == 1;
    LoadMotor(path, section, L"LT", p.lt, dEn, dDur, dLt);
    LoadMotor(path, section, L"RT", p.rt, dEn, dDur, dRt);
    LoadMotor(path, section, L"LM", p.lm, dEn, dDur, dLm);
    LoadMotor(path, section, L"RM", p.rm, dEn, dDur, dRm);
}

// Save a full profile to the INI.
void Config::SaveProfile(const std::wstring& path, const std::wstring& section, const VibeProfile& p) {
    WritePrivateProfileStringW(section.c_str(), L"Enabled", p.enabled ? L"1" : L"0", path.c_str());
    SaveMotor(path, section, L"LT", p.lt);
    SaveMotor(path, section, L"RT", p.rt);
    SaveMotor(path, section, L"LM", p.lm);
    SaveMotor(path, section, L"RM", p.rm);
}

void Config::Load() {
    std::wstring path = GetConfigPath();
    m_settings.showDebugHUD = GetPrivateProfileIntW(L"Global", L"ShowHUD", 0, path.c_str()) == 1;

    // Define all profiles with their defaults
    std::vector<std::tuple<std::wstring, VibeProfile&, ProfileDefaults>> profiles = {
        // Vehicle Physics
        {L"GearShift", m_settings.gearShift, {true, true, 150, 0.5f, 1.0f, 0.0f, 0.0f}},
        {L"ABS", m_settings.abs, {true, true, 100, 1.0f, 0.0f, 0.0f, 0.0f}},
        {L"TractionLoss", m_settings.tractionLoss, {true, true, 100, 0.0f, 0.7f, 0.0f, 0.0f}},
        {L"Drifting", m_settings.drifting, {true, true, 100, 0.0f, 0.6f, 0.0f, 0.6f}},
        {L"SuspensionBump", m_settings.suspensionBump, {true, true, 100, 0.0f, 0.0f, 0.7f, 0.7f}},
        {L"Collision", m_settings.collision, {true, true, 300, 1.0f, 1.0f, 1.0f, 1.0f}},
        {L"EngineStutter", m_settings.engineStutter, {true, true, 100, 0.0f, 0.6f, 0.0f, 0.0f}},
        {L"EngineRPM", m_settings.engineRPM, {true, true, 0, 0.0f, 0.0f, 0.0f, 0.3f}},
        {L"DamageTaken", m_settings.damageTaken, {true, true, 200, 1.0f, 1.0f, 0.8f, 0.0f}},

        // Combat
        {L"Aiming", m_settings.aiming, {true, true, 50, 0.05f, 0.0f, 1.0f, 0.0f}},
        {L"Shoot_Other", m_settings.shootOther, {true, true, 250, 0.0f, 1.0f, 1.0f, 1.0f}},

        // New Events
        {L"Melee_Impact", m_settings.meleeImpact, {true, true, 150, 0.0f, 0.8f, 0.6f, 0.0f}},
        {L"Reloading", m_settings.reloading, {true, true, 100, 0.3f, 0.3f, 0.0f, 0.0f}},
        {L"Weapon_Switch", m_settings.weaponSwitch, {true, true, 80, 0.5f, 0.5f, 0.0f, 0.0f}},
        {L"Getting_Shot", m_settings.gettingShot, {true, true, 200, 1.0f, 1.0f, 0.8f, 0.0f}},
        {L"Explosion", m_settings.explosion, {true, true, 400, 1.0f, 1.0f, 1.0f, 1.0f}},
        {L"Blast_Wave", m_settings.blastWave, {true, true, 300, 0.8f, 0.8f, 0.8f, 0.8f}},
        {L"Electrocuted", m_settings.electrocuted, {true, true, 200, 0.7f, 0.7f, 0.0f, 0.0f}},
        {L"Burning", m_settings.burning, {true, true, 150, 0.6f, 0.0f, 0.5f, 0.0f}},

        // Vehicle Damage & Types
        {L"Tire_Burst", m_settings.tireBurst, {true, true, 200, 0.0f, 0.0f, 0.8f, 0.8f}},
        {L"Vehicle_On_Fire", m_settings.vehicleOnFire, {true, true, 0, 0.4f, 0.0f, 0.3f, 0.0f}},
        {L"Helicopter_Blades", m_settings.helicopterBlades, {true, true, 0, 0.0f, 0.0f, 0.3f, 0.3f}},
        {L"Train_Vibration", m_settings.trainVibration, {true, true, 0, 0.0f, 0.0f, 0.2f, 0.2f}},
        {L"Airplane_Turbulence", m_settings.airplaneTurbulence, {true, true, 0, 0.2f, 0.0f, 0.2f, 0.0f}},

        // Player Movement
        {L"Climbing", m_settings.climbing, {true, true, 0, 0.0f, 0.0f, 0.3f, 0.0f}},
        {L"Jumping", m_settings.jumping, {true, true, 100, 0.0f, 0.0f, 0.7f, 0.7f}},
        {L"Falling_Impact", m_settings.fallingImpact, {true, true, 200, 0.0f, 0.0f, 1.0f, 1.0f}},
        {L"Swimming", m_settings.swimming, {true, true, 0, 0.1f, 0.0f, 0.1f, 0.0f}},
        {L"Parachuting", m_settings.parachuting, {true, true, 0, 0.0f, 0.0f, 0.2f, 0.0f}},
        {L"Sliding", m_settings.sliding, {true, true, 0, 0.0f, 0.0f, 0.1f, 0.1f}},

        // Environmental Events
        {L"Lightning_Strike", m_settings.lightningStrike, {true, true, 500, 1.0f, 1.0f, 1.0f, 1.0f}},
        {L"Earthquake", m_settings.earthquake, {true, true, 0, 0.3f, 0.3f, 0.3f, 0.3f}},

        // Player Status
        {L"Police_Chase", m_settings.policeChase, {true, true, 0, 0.5f, 0.0f, 0.3f, 0.0f}},
        {L"Stunned", m_settings.stunned, {true, true, 0, 0.4f, 0.4f, 0.0f, 0.0f}},
        {L"Drowning", m_settings.drowning, {true, true, 0, 0.3f, 0.3f, 0.0f, 0.0f}},
        {L"Poisoned", m_settings.poisoned, {true, true, 0, 0.6f, 0.0f, 0.0f, 0.0f}}
    };

    // Load all profiles
    for (auto& [section, profile, defaults] : profiles) {
        LoadProfile(path, section, profile, defaults.enabled, defaults.en, defaults.dur, defaults.lt, defaults.rt, defaults.lm, defaults.rm);
    }

    // Special overrides for Aiming
    m_settings.aiming.lt.dur = GetPrivateProfileIntW(L"Aiming", L"LT_Dur", 50, path.c_str());
    m_settings.aiming.lm.dur = GetPrivateProfileIntW(L"Aiming", L"LM_Dur", 100, path.c_str());

    // Special overrides for Shoot_Other
    m_settings.shootOther.rt.dur = GetPrivateProfileIntW(L"Shoot_Other", L"RT_Dur", 200, path.c_str());
    m_settings.shootOther.lm.dur = GetPrivateProfileIntW(L"Shoot_Other", L"LM_Dur", 250, path.c_str());
    m_settings.shootOther.rm.dur = GetPrivateProfileIntW(L"Shoot_Other", L"RM_Dur", 250, path.c_str());
    m_settings.shootOther.rt.val = GetPrivateProfileIntW(L"Shoot_Other", L"RT_Val", 100, path.c_str()) / 100.0f;
    m_settings.shootOther.lm.val = GetPrivateProfileIntW(L"Shoot_Other", L"LM_Val", 100, path.c_str()) / 100.0f;
    m_settings.shootOther.rm.val = GetPrivateProfileIntW(L"Shoot_Other", L"RM_Val", 100, path.c_str()) / 100.0f;
}

void Config::Save() {
    std::wstring path = GetConfigPath();
    WritePrivateProfileStringW(L"Global", L"ShowHUD", m_settings.showDebugHUD ? L"1" : L"0", path.c_str());

    // List of all profiles to save
    std::vector<std::pair<std::wstring, const VibeProfile&>> profilesToSave = {
        {L"GearShift", m_settings.gearShift},
        {L"ABS", m_settings.abs},
        {L"TractionLoss", m_settings.tractionLoss},
        {L"Drifting", m_settings.drifting},
        {L"SuspensionBump", m_settings.suspensionBump},
        {L"Collision", m_settings.collision},
        {L"EngineStutter", m_settings.engineStutter},
        {L"EngineRPM", m_settings.engineRPM},
        {L"DamageTaken", m_settings.damageTaken},
        {L"Aiming", m_settings.aiming},
        {L"Shoot_Other", m_settings.shootOther},
        {L"Melee_Impact", m_settings.meleeImpact},
        {L"Reloading", m_settings.reloading},
        {L"Weapon_Switch", m_settings.weaponSwitch},
        {L"Getting_Shot", m_settings.gettingShot},
        {L"Explosion", m_settings.explosion},
        {L"Blast_Wave", m_settings.blastWave},
        {L"Electrocuted", m_settings.electrocuted},
        {L"Burning", m_settings.burning},
        {L"Tire_Burst", m_settings.tireBurst},
        {L"Vehicle_On_Fire", m_settings.vehicleOnFire},
        {L"Helicopter_Blades", m_settings.helicopterBlades},
        {L"Train_Vibration", m_settings.trainVibration},
        {L"Airplane_Turbulence", m_settings.airplaneTurbulence},
        {L"Climbing", m_settings.climbing},
        {L"Jumping", m_settings.jumping},
        {L"Falling_Impact", m_settings.fallingImpact},
        {L"Swimming", m_settings.swimming},
        {L"Parachuting", m_settings.parachuting},
        {L"Sliding", m_settings.sliding},
        {L"Lightning_Strike", m_settings.lightningStrike},
        {L"Earthquake", m_settings.earthquake},
        {L"Police_Chase", m_settings.policeChase},
        {L"Stunned", m_settings.stunned},
        {L"Drowning", m_settings.drowning},
        {L"Poisoned", m_settings.poisoned}
    };

    // Save all profiles
    for (auto& [section, profile] : profilesToSave) {
        SaveProfile(path, section, profile);
    }
}

Settings& Config::Get() {
    return m_settings;
}