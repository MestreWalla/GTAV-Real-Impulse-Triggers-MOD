#pragma once
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include "Telemetry.h"
#include "Config.h"

enum class MenuState {
    Main,
    EventSelect,
    EventEdit
};

class Menu {
public:
    static void Update(bool controllerConnected);
    static void Draw(bool controllerConnected);
    static void DrawHUD(const VehicleTelemetry& data);
    static void Log(const std::string& message, const std::string& colorCode = "");
    static bool IsOpen() { return m_isOpen; }
    static int GetManualTestEvent() { return m_manualTestEvent; }
    static void ClearManualTest() { m_manualTestEvent = -1; }

private:
    static bool m_isOpen;
    static int m_selectedIndex;
    static int m_selectedEventIndex;
    static int m_manualTestEvent;
    static MenuState m_state;
    static std::vector<std::string> m_logs;
    static std::chrono::steady_clock::time_point m_testStartTime;

    static void RenderText(const std::string& text, float x, float y, float scale, bool selected);
    static VibeProfile* GetSelectedProfile();
    static std::string GetEventName(int index);
};