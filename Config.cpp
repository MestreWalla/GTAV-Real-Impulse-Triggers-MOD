#include "Config.h"
#include <windows.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

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

    // Default configs                                              Enb  Dur   LT    RT    LM    RM
    LoadProfile(path, L"GearShift", m_settings.gearShift,           true, true, 150, 0.5f, 1.0f, 0.0f, 0.0f);
    LoadProfile(path, L"ABS", m_settings.abs,                       true, true, 100, 1.0f, 0.0f, 0.0f, 0.0f);
    LoadProfile(path, L"TractionLoss", m_settings.tractionLoss,     true, true, 100, 0.0f, 0.7f, 0.0f, 0.0f);
    LoadProfile(path, L"Drifting", m_settings.drifting,             true, true, 100, 0.0f, 0.6f, 0.0f, 0.6f);
    LoadProfile(path, L"SuspensionBump", m_settings.suspensionBump, true, true, 100, 0.0f, 0.0f, 0.7f, 0.7f);
    LoadProfile(path, L"Collision", m_settings.collision,           true, true, 300, 1.0f, 1.0f, 1.0f, 1.0f);
    LoadProfile(path, L"EngineStutter", m_settings.engineStutter,   true, true, 100, 0.0f, 0.6f, 0.0f, 0.0f);
    LoadProfile(path, L"EngineRPM", m_settings.engineRPM,           true, true, 0,   0.0f, 0.0f, 0.0f, 0.3f);
    LoadProfile(path, L"DamageTaken", m_settings.damageTaken,       true, true, 200, 1.0f, 1.0f, 0.8f, 0.0f);
    // Aiming: LT pulse only, short duration.
    LoadProfile(path, L"Aiming", m_settings.aiming,                 true, true, 50, 0.05f, 0.0f, 1.0f, 0.0f);
    m_settings.aiming.lt.dur = GetPrivateProfileIntW(L"Aiming", L"LT_Dur", 50, path.c_str());
    m_settings.aiming.lm.dur = GetPrivateProfileIntW(L"Aiming", L"LM_Dur", 100, path.c_str());

    // Shoot_Other: RT full-power pulse + LM/RM full-power support.
    LoadProfile(path, L"Shoot_Other", m_settings.shootOther,        true, true, 250, 0.0f, 1.0f, 1.0f, 1.0f);
    m_settings.shootOther.rt.dur = GetPrivateProfileIntW(L"Shoot_Other", L"RT_Dur", 200, path.c_str());
    m_settings.shootOther.lm.dur = GetPrivateProfileIntW(L"Shoot_Other", L"LM_Dur", 250, path.c_str());
    m_settings.shootOther.rm.dur = GetPrivateProfileIntW(L"Shoot_Other", L"RM_Dur", 250, path.c_str());
    m_settings.shootOther.rt.val = GetPrivateProfileIntW(L"Shoot_Other", L"RT_Val", 100, path.c_str()) / 100.0f;
    m_settings.shootOther.lm.val = GetPrivateProfileIntW(L"Shoot_Other", L"LM_Val", 100, path.c_str()) / 100.0f;
    m_settings.shootOther.rm.val = GetPrivateProfileIntW(L"Shoot_Other", L"RM_Val", 100, path.c_str()) / 100.0f;
    
    // New Events
    LoadProfile(path, L"Melee_Impact", m_settings.meleeImpact,      true, true, 150, 0.0f, 0.8f, 0.6f, 0.0f);
    LoadProfile(path, L"Reloading", m_settings.reloading,           true, true, 100, 0.3f, 0.3f, 0.0f, 0.0f);
    LoadProfile(path, L"Weapon_Switch", m_settings.weaponSwitch,    true, true, 80,  0.5f, 0.5f, 0.0f, 0.0f);
    LoadProfile(path, L"Getting_Shot", m_settings.gettingShot,      true, true, 200, 1.0f, 1.0f, 0.8f, 0.0f);
    LoadProfile(path, L"Explosion", m_settings.explosion,           true, true, 400, 1.0f, 1.0f, 1.0f, 1.0f);
    LoadProfile(path, L"Blast_Wave", m_settings.blastWave,          true, true, 300, 0.8f, 0.8f, 0.8f, 0.8f);
    LoadProfile(path, L"Electrocuted", m_settings.electrocuted,     true, true, 200, 0.7f, 0.7f, 0.0f, 0.0f);
    LoadProfile(path, L"Burning", m_settings.burning,               true, true, 150, 0.6f, 0.0f, 0.5f, 0.0f);
    LoadProfile(path, L"Tire_Burst", m_settings.tireBurst,          true, true, 200, 0.0f, 0.0f, 0.8f, 0.8f);
    LoadProfile(path, L"Vehicle_On_Fire", m_settings.vehicleOnFire, true, true, 0,   0.4f, 0.0f, 0.3f, 0.0f);
    LoadProfile(path, L"Helicopter_Blades", m_settings.helicopterBlades, true, true, 0, 0.0f, 0.0f, 0.3f, 0.3f);
    LoadProfile(path, L"Train_Vibration", m_settings.trainVibration, true, true, 0,  0.0f, 0.0f, 0.2f, 0.2f);
    LoadProfile(path, L"Airplane_Turbulence", m_settings.airplaneTurbulence, true, true, 0, 0.2f, 0.0f, 0.2f, 0.0f);
    LoadProfile(path, L"Climbing", m_settings.climbing,             true, true, 0,   0.0f, 0.0f, 0.3f, 0.0f);
    LoadProfile(path, L"Jumping", m_settings.jumping,               true, true, 100, 0.0f, 0.0f, 0.7f, 0.7f);
    LoadProfile(path, L"Falling_Impact", m_settings.fallingImpact,  true, true, 200, 0.0f, 0.0f, 1.0f, 1.0f);
    LoadProfile(path, L"Swimming", m_settings.swimming,             true, true, 0,   0.1f, 0.0f, 0.1f, 0.0f);
    LoadProfile(path, L"Parachuting", m_settings.parachuting,       true, true, 0,   0.0f, 0.0f, 0.2f, 0.0f);
    LoadProfile(path, L"Sliding", m_settings.sliding,               true, true, 0,   0.0f, 0.0f, 0.1f, 0.1f);
    LoadProfile(path, L"Lightning_Strike", m_settings.lightningStrike, true, true, 500, 1.0f, 1.0f, 1.0f, 1.0f);
    LoadProfile(path, L"Earthquake", m_settings.earthquake,         true, true, 0,   0.3f, 0.3f, 0.3f, 0.3f);
    LoadProfile(path, L"Police_Chase", m_settings.policeChase,      true, true, 0,   0.5f, 0.0f, 0.3f, 0.0f);
    LoadProfile(path, L"Stunned", m_settings.stunned,               true, true, 0,   0.4f, 0.4f, 0.0f, 0.0f);
    LoadProfile(path, L"Drowning", m_settings.drowning,             true, true, 0,   0.3f, 0.3f, 0.0f, 0.0f);
    LoadProfile(path, L"Poisoned", m_settings.poisoned,             true, true, 0,   0.6f, 0.0f, 0.0f, 0.0f);
}

void Config::Save() {
    std::wstring path = GetConfigPath();
    WritePrivateProfileStringW(L"Global", L"ShowHUD", m_settings.showDebugHUD ? L"1" : L"0", path.c_str());

    SaveProfile(path, L"GearShift", m_settings.gearShift);
    SaveProfile(path, L"ABS", m_settings.abs);
    SaveProfile(path, L"TractionLoss", m_settings.tractionLoss);
    SaveProfile(path, L"Drifting", m_settings.drifting);
    SaveProfile(path, L"SuspensionBump", m_settings.suspensionBump);
    SaveProfile(path, L"Collision", m_settings.collision);
    SaveProfile(path, L"EngineStutter", m_settings.engineStutter);
    SaveProfile(path, L"EngineRPM", m_settings.engineRPM);
    SaveProfile(path, L"DamageTaken", m_settings.damageTaken);
    SaveProfile(path, L"Aiming", m_settings.aiming);
    SaveProfile(path, L"Shoot_Other", m_settings.shootOther);
    
    // New Events
    SaveProfile(path, L"Melee_Impact", m_settings.meleeImpact);
    SaveProfile(path, L"Reloading", m_settings.reloading);
    SaveProfile(path, L"Weapon_Switch", m_settings.weaponSwitch);
    SaveProfile(path, L"Getting_Shot", m_settings.gettingShot);
    SaveProfile(path, L"Explosion", m_settings.explosion);
    SaveProfile(path, L"Blast_Wave", m_settings.blastWave);
    SaveProfile(path, L"Electrocuted", m_settings.electrocuted);
    SaveProfile(path, L"Burning", m_settings.burning);
    SaveProfile(path, L"Tire_Burst", m_settings.tireBurst);
    SaveProfile(path, L"Vehicle_On_Fire", m_settings.vehicleOnFire);
    SaveProfile(path, L"Helicopter_Blades", m_settings.helicopterBlades);
    SaveProfile(path, L"Train_Vibration", m_settings.trainVibration);
    SaveProfile(path, L"Airplane_Turbulence", m_settings.airplaneTurbulence);
    SaveProfile(path, L"Climbing", m_settings.climbing);
    SaveProfile(path, L"Jumping", m_settings.jumping);
    SaveProfile(path, L"Falling_Impact", m_settings.fallingImpact);
    SaveProfile(path, L"Swimming", m_settings.swimming);
    SaveProfile(path, L"Parachuting", m_settings.parachuting);
    SaveProfile(path, L"Sliding", m_settings.sliding);
    SaveProfile(path, L"Lightning_Strike", m_settings.lightningStrike);
    SaveProfile(path, L"Earthquake", m_settings.earthquake);
    SaveProfile(path, L"Police_Chase", m_settings.policeChase);
    SaveProfile(path, L"Stunned", m_settings.stunned);
    SaveProfile(path, L"Drowning", m_settings.drowning);
    SaveProfile(path, L"Poisoned", m_settings.poisoned);
}

Settings& Config::Get() {
    return m_settings;
}