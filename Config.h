#pragma once
#include <string>

struct MotorConfig {
    bool en = true;
    int dur = 100;
    float val = 0.0f;
};

struct VibeProfile {
    MotorConfig lt; // Left Trigger
    MotorConfig rt; // Right Trigger
    MotorConfig lm; // Left Motor (Main)
    MotorConfig rm; // Right Motor (Main)
};

struct Settings {
    bool showDebugHUD = false;
    
    // Profiles
    VibeProfile gearShift;
    VibeProfile abs;
    VibeProfile tractionLoss;
    VibeProfile drifting;
    VibeProfile suspensionBump;
    VibeProfile collision;
    VibeProfile engineStutter;
    VibeProfile engineRPM;
    VibeProfile damageTaken;
    VibeProfile aiming;
    VibeProfile shootPistol;
    VibeProfile shootSMG;
    VibeProfile shootShotgun;
    VibeProfile shootHeavy;
    VibeProfile shootOther;
};

class Config {
public:
    static void Load();
    static void Save();
    static void Reload() { Load(); }
    static Settings& Get();

private:
    static Settings m_settings;
    static std::wstring GetConfigPath();
    static void LoadMotor(const std::wstring& path, const std::wstring& section, const std::wstring& prefix, MotorConfig& m, bool dEn, int dDur, float dVal);
    static void SaveMotor(const std::wstring& path, const std::wstring& section, const std::wstring& prefix, const MotorConfig& m);
    static void LoadProfile(const std::wstring& path, const std::wstring& section, VibeProfile& p, bool dEn, int dDur, float dLt, float dRt, float dLm, float dRm);
    static void SaveProfile(const std::wstring& path, const std::wstring& section, const VibeProfile& p);
};