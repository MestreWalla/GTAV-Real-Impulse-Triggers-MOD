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

void Config::LoadMotor(const std::wstring& path, const std::wstring& section, const std::wstring& prefix, MotorConfig& m, bool dEn, int dDur, float dVal) {
    m.en = GetPrivateProfileIntW(section.c_str(), (prefix + L"_En").c_str(), dEn ? 1 : 0, path.c_str()) == 1;
    m.dur = GetPrivateProfileIntW(section.c_str(), (prefix + L"_Dur").c_str(), dDur, path.c_str());
    m.val = (float)GetPrivateProfileIntW(section.c_str(), (prefix + L"_Val").c_str(), (int)(dVal * 100), path.c_str()) / 100.0f;
}

void Config::SaveMotor(const std::wstring& path, const std::wstring& section, const std::wstring& prefix, const MotorConfig& m) {
    WritePrivateProfileStringW(section.c_str(), (prefix + L"_En").c_str(), m.en ? L"1" : L"0", path.c_str());
    WritePrivateProfileStringW(section.c_str(), (prefix + L"_Dur").c_str(), std::to_wstring(m.dur).c_str(), path.c_str());
    WritePrivateProfileStringW(section.c_str(), (prefix + L"_Val").c_str(), std::to_wstring((int)(m.val * 100)).c_str(), path.c_str());
}

void Config::LoadProfile(const std::wstring& path, const std::wstring& section, VibeProfile& p, bool dEn, int dDur, float dLt, float dRt, float dLm, float dRm) {
    LoadMotor(path, section, L"LT", p.lt, dEn, dDur, dLt);
    LoadMotor(path, section, L"RT", p.rt, dEn, dDur, dRt);
    LoadMotor(path, section, L"LM", p.lm, dEn, dDur, dLm);
    LoadMotor(path, section, L"RM", p.rm, dEn, dDur, dRm);
}

void Config::SaveProfile(const std::wstring& path, const std::wstring& section, const VibeProfile& p) {
    SaveMotor(path, section, L"LT", p.lt);
    SaveMotor(path, section, L"RT", p.rt);
    SaveMotor(path, section, L"LM", p.lm);
    SaveMotor(path, section, L"RM", p.rm);
}

void Config::Load() {
    std::wstring path = GetConfigPath();
    m_settings.showDebugHUD = GetPrivateProfileIntW(L"Global", L"ShowHUD", 0, path.c_str()) == 1;

    // Default configs                                              Enb  Dur  LT    RT    LM    RM
    LoadProfile(path, L"GearShift", m_settings.gearShift,           true, 150, 0.5f, 1.0f, 0.0f, 0.0f);
    LoadProfile(path, L"ABS", m_settings.abs,                       true, 100, 1.0f, 0.0f, 0.0f, 0.0f);
    LoadProfile(path, L"TractionLoss", m_settings.tractionLoss,     true, 100, 0.0f, 0.7f, 0.0f, 0.0f);
    LoadProfile(path, L"Drifting", m_settings.drifting,             true, 100, 0.0f, 0.6f, 0.0f, 0.6f);
    LoadProfile(path, L"SuspensionBump", m_settings.suspensionBump, true, 100, 0.0f, 0.0f, 0.7f, 0.7f);
    LoadProfile(path, L"Collision", m_settings.collision,           true, 300, 1.0f, 1.0f, 1.0f, 1.0f);
    LoadProfile(path, L"EngineStutter", m_settings.engineStutter,   true, 100, 0.0f, 0.6f, 0.0f, 0.0f);
    LoadProfile(path, L"EngineRPM", m_settings.engineRPM,           true, 0,   0.0f, 0.0f, 0.0f, 0.3f);
    LoadProfile(path, L"DamageTaken", m_settings.damageTaken,       true, 200, 1.0f, 1.0f, 0.8f, 0.0f);
    LoadProfile(path, L"Aiming", m_settings.aiming,                 true, 0,   0.15f, 0.0f, 0.05f, 0.0f);
    LoadProfile(path, L"Shoot_Pistol", m_settings.shootPistol,      true, 100, 0.0f, 0.8f, 0.0f, 0.2f);
    LoadProfile(path, L"Shoot_SMG", m_settings.shootSMG,            true, 80,  0.0f, 0.6f, 0.0f, 0.1f);
    LoadProfile(path, L"Shoot_Shotgun", m_settings.shootShotgun,    true, 150, 0.0f, 1.0f, 0.0f, 0.6f);
    LoadProfile(path, L"Shoot_Heavy", m_settings.shootHeavy,        true, 180, 0.0f, 1.0f, 0.0f, 0.8f);
    LoadProfile(path, L"Shoot_Other", m_settings.shootOther,        true, 100, 0.0f, 0.8f, 0.0f, 0.4f);
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
    SaveProfile(path, L"Shoot_Pistol", m_settings.shootPistol);
    SaveProfile(path, L"Shoot_SMG", m_settings.shootSMG);
    SaveProfile(path, L"Shoot_Shotgun", m_settings.shootShotgun);
    SaveProfile(path, L"Shoot_Heavy", m_settings.shootHeavy);
    SaveProfile(path, L"Shoot_Other", m_settings.shootOther);
}

Settings& Config::Get() {
    return m_settings;
}