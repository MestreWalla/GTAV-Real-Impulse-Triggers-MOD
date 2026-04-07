#pragma once
#include <string>

struct MotorConfig {
    bool en = true;
    int dur = 100;
    float val = 0.0f;
};

struct VibeProfile {
    bool enabled = true;
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
    VibeProfile shootOther;
    
    // Combat/Weapon events
    VibeProfile meleeImpact;
    VibeProfile reloading;
    VibeProfile weaponSwitch;
    VibeProfile gettingShot;
    VibeProfile explosion;
    VibeProfile blastWave;
    VibeProfile electrocuted;
    VibeProfile burning;
    
    // Vehicle events
    VibeProfile tireBurst;
    VibeProfile vehicleExplosion;
    VibeProfile vehicleOnFire;
    VibeProfile windowBreak;
    VibeProfile helicopterBlades;
    VibeProfile trainVibration;
    VibeProfile airplaneTurbulence;
    
    // Movement events
    VibeProfile climbing;
    VibeProfile jumping;
    VibeProfile fallingImpact;
    VibeProfile swimming;
    VibeProfile parachuting;
    VibeProfile sliding;
    
    // Environmental events
    VibeProfile lightningStrike;
    VibeProfile earthquake;
    VibeProfile policeChase;
    
    // General events
    VibeProfile stunned;
    VibeProfile drowning;
    VibeProfile poisoned;
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
    static void LoadProfile(const std::wstring& path, const std::wstring& section, VibeProfile& p, bool dEnabled, bool dEn, int dDur, float dLt, float dRt, float dLm, float dRm);
    static void SaveProfile(const std::wstring& path, const std::wstring& section, const VibeProfile& p);
};