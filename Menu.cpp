#include "Menu.h"
#include "Config.h"
#include "inc/natives.h"
#include <vector>
#include <string>

bool Menu::m_isOpen = false;
int Menu::m_selectedIndex = 0;
int Menu::m_selectedEventIndex = 0;
int Menu::m_manualTestEvent = -1;
MenuState Menu::m_state = MenuState::Main;
std::vector<std::string> Menu::m_logs;
std::chrono::steady_clock::time_point Menu::m_testStartTime;

void Menu::RenderText(const std::string& text, float x, float y, float scale, bool selected) {
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, scale);
    if (selected) {
        UI::SET_TEXT_COLOUR(0, 0, 0, 255);
    } else {
        UI::SET_TEXT_COLOUR(255, 255, 255, 255);
    }
    UI::SET_TEXT_WRAP(0.0f, 1.0f);
    UI::SET_TEXT_CENTRE(0);
    UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING((char*)text.c_str());
    UI::_DRAW_TEXT(x, y);
}

void Menu::Log(const std::string& message, const std::string& colorCode) {
    std::string formatted = colorCode + message;
    if (m_logs.size() > 10) m_logs.erase(m_logs.begin());
    m_logs.push_back(formatted);
}

std::string Menu::GetEventName(int index) {
    std::vector<std::string> names = {
        "GearShift", "ABS", "TractionLoss", "Drifting", "SuspensionBump",
        "Collision", "EngineStutter", "EngineRPM", "DamageTaken", "Aiming",
        "Shoot_Pistol", "Shoot_SMG", "Shoot_Shotgun", "Shoot_Heavy", "Shoot_Other"
    };
    if (index >= 0 && index < (int)names.size()) return names[index];
    return "Unknown";
}

VibeProfile* Menu::GetSelectedProfile() {
    Settings& s = Config::Get();
    switch(m_selectedEventIndex) {
        case 0: return &s.gearShift; case 1: return &s.abs; case 2: return &s.tractionLoss;
        case 3: return &s.drifting; case 4: return &s.suspensionBump; case 5: return &s.collision;
        case 6: return &s.engineStutter; case 7: return &s.engineRPM; case 8: return &s.damageTaken;
        case 9: return &s.aiming; case 10: return &s.shootPistol; case 11: return &s.shootSMG;
        case 12: return &s.shootShotgun; case 13: return &s.shootHeavy; case 14: return &s.shootOther;
    }
    return nullptr;
}

void Menu::Update(bool connected) {
    if (GetAsyncKeyState(VK_F5) & 1) m_isOpen = !m_isOpen;
    if (!m_isOpen) return;

    if (GetAsyncKeyState(VK_UP) & 1) m_selectedIndex--;
    if (GetAsyncKeyState(VK_DOWN) & 1) m_selectedIndex++;
    if (GetAsyncKeyState(VK_BACK) & 1) {
        if (m_state == MenuState::EventSelect) m_state = MenuState::Main;
        else if (m_state == MenuState::EventEdit) m_state = MenuState::EventSelect;
        m_selectedIndex = 0;
    }

    Settings& s = Config::Get();
    int limit = 0;

    if (m_state == MenuState::Main) {
        limit = 3;
        if (m_selectedIndex < 0) m_selectedIndex = limit - 1;
        if (m_selectedIndex >= limit) m_selectedIndex = 0;
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            switch(m_selectedIndex) {
                case 0: s.showDebugHUD = !s.showDebugHUD; break;
                case 1: m_state = MenuState::EventSelect; m_selectedIndex = 0; break;
                case 2: Config::Save(); Log("SYSTEM: SETTINGS_SAVED", "~g~"); break;
            }
        }
    } 
    else if (m_state == MenuState::EventSelect) {
        limit = 15;
        if (m_selectedIndex < 0) m_selectedIndex = limit - 1;
        if (m_selectedIndex >= limit) m_selectedIndex = 0;
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            m_selectedEventIndex = m_selectedIndex;
            m_state = MenuState::EventEdit;
            m_selectedIndex = 0;
        }
    }
    else if (m_state == MenuState::EventEdit) {
        limit = 14; // 12 params + Save + Test
        if (m_selectedIndex < 0) m_selectedIndex = limit - 1;
        if (m_selectedIndex >= limit) m_selectedIndex = 0;
        
        VibeProfile* p = GetSelectedProfile();
        if (p) {
            bool left = GetAsyncKeyState(VK_LEFT) & 1;
            bool right = GetAsyncKeyState(VK_RIGHT) & 1;
            bool enter = GetAsyncKeyState(VK_RETURN) & 1;

            if (m_selectedIndex == 12) { // SAVE
                if (enter) { Config::Save(); Log("SYSTEM: EVENT_SAVED", "~g~"); }
            } 
            else if (m_selectedIndex == 13) { // TEST
                if (enter) m_manualTestEvent = m_selectedEventIndex;
            }
            else {
                int motorIdx = m_selectedIndex / 3;
                int paramIdx = m_selectedIndex % 3;
                MotorConfig* mc = (motorIdx == 0 ? &p->lt : (motorIdx == 1 ? &p->rt : (motorIdx == 2 ? &p->lm : &p->rm)));
                
                if (paramIdx == 0 && enter) mc->en = !mc->en;
                if (paramIdx == 1) { // Power
                    if (left) mc->val -= 0.05f; if (right) mc->val += 0.05f;
                    if (mc->val < 0) mc->val = 0; if (mc->val > 1) mc->val = 1;
                }
                if (paramIdx == 2) { // Time
                    if (left) mc->dur -= 50; if (right) mc->dur += 50;
                    if (mc->dur < 0) mc->dur = 0;
                }
            }
        }
    }

    Draw(connected);
}

void Menu::Draw(bool connected) {
    float menuX = 0.12f, menuY = 0.35f, menuW = 0.18f, menuH = 0.65f;
    GRAPHICS::DRAW_RECT(menuX, menuY, menuW, menuH, 0, 0, 0, 200); 
    GRAPHICS::DRAW_RECT(menuX, 0.08f, menuW, 0.05f, 255, 255, 0, 200); 
    RenderText("IMPULSE TRIGGERS", menuX - 0.08f, 0.065f, 0.5f, false);

    std::string status = "Status: " + std::string(connected ? "~g~CONNECTED" : "~r~DISCONNECTED");
    RenderText(status, menuX - 0.08f, 0.11f, 0.35f, false);

    std::vector<std::string> options;
    float startY = 0.15f;

    if (m_state == MenuState::Main) {
        options = { "HUD: " + std::string(Config::Get().showDebugHUD ? "ON" : "OFF"), "Edit Game Events >", "SAVE ALL CONFIG" };
    } else if (m_state == MenuState::EventSelect) {
        for (int i=0; i<15; i++) options.push_back(GetEventName(i));
    } else if (m_state == MenuState::EventEdit) {
        // Render Title separately so it doesn't shift indices
        RenderText("--- " + GetEventName(m_selectedEventIndex) + " ---", menuX - 0.08f, 0.15f, 0.35f, false);
        startY = 0.185f; // Start options lower

        VibeProfile* p = GetSelectedProfile();
        std::vector<std::string> mNames = {"LT", "RT", "LM", "RM"};
        for (int i=0; i<4; i++) {
            MotorConfig* mc = (i==0?&p->lt:(i==1?&p->rt:(i==2?&p->lm:&p->rm)));
            options.push_back(mNames[i] + ": " + (mc->en?"~g~EN":"~r~DIS"));
            options.push_back("  Power: " + std::to_string((int)(mc->val*100)) + "%");
            options.push_back("  Time: " + std::to_string(mc->dur) + "ms");
        }
        options.push_back("~g~SAVE THIS EVENT");
        options.push_back("~y~TEST THIS EVENT");
    }

    for (int i = 0; i < (int)options.size(); i++) {
        float yPos = startY + (i * 0.035f);
        if (m_selectedIndex == i) {
            GRAPHICS::DRAW_RECT(menuX, yPos + 0.015f, menuW, 0.032f, 255, 255, 255, 255); 
            RenderText("> " + options[i], menuX - 0.08f, yPos, 0.35f, true);
        } else {
            RenderText(options[i], menuX - 0.08f, yPos, 0.35f, false);
        }
    }
}

void Menu::DrawHUD(const VehicleTelemetry& data) {
    GRAPHICS::DRAW_RECT(0.88f, 0.15f, 0.18f, 0.15f, 0, 0, 0, 150);
    RenderText("VEHICLE INFO", 0.8f, 0.08f, 0.4f, false);
    if (data.isInVehicle) {
        RenderText("Speed: " + std::to_string((int)(data.vehicleSpeed * 3.6f)) + " km/h", 0.8f, 0.12f, 0.3f, false);
        RenderText("Gear: " + std::to_string(data.gear) + " | RPM: " + std::to_string((int)(data.rpm * 100)) + "%", 0.8f, 0.145f, 0.3f, false);
        RenderText("Health: " + std::to_string((int)data.engineHealth), 0.8f, 0.17f, 0.3f, (data.engineHealth < 400.0f));
    } else RenderText("ON FOOT", 0.8f, 0.12f, 0.3f, false);

    float logStartY = 0.65f;
    float logHeight = m_logs.size() * 0.025f + 0.05f;
    GRAPHICS::DRAW_RECT(0.88f, logStartY + (logHeight/2.0f) - 0.025f, 0.18f, logHeight, 0, 0, 0, 180);
    RenderText("SIGNAL LOG", 0.8f, logStartY - 0.02f, 0.4f, false);
    for (int i = 0; i < (int)m_logs.size(); i++) RenderText(m_logs[i], 0.8f, logStartY + (i * 0.025f), 0.28f, false);
}