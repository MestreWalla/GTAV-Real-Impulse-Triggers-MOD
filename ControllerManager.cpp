#include "ControllerManager.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <algorithm>

using namespace winrt;
using namespace Windows::Gaming::Input;

ControllerManager::ControllerManager() {
    m_addedToken = Gamepad::GamepadAdded([this](auto&& sender, Gamepad const& args) { OnGamepadAdded(sender, args); });
    m_removedToken = Gamepad::GamepadRemoved([this](auto&& sender, Gamepad const& args) { OnGamepadRemoved(sender, args); });

    for (int i = 0; i < 5; i++) {
        auto gamepads = Gamepad::Gamepads();
        if (gamepads.Size() > 0) {
            m_gamepad = gamepads.GetAt(0);
            m_isConnected = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

ControllerManager::~ControllerManager() {
    Gamepad::GamepadAdded(m_addedToken);
    Gamepad::GamepadRemoved(m_removedToken);
}

void ControllerManager::OnGamepadAdded(winrt::Windows::Foundation::IInspectable const& sender, Gamepad const& args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_gamepad == nullptr) {
        m_gamepad = args;
        m_isConnected = true;
    }
}

void ControllerManager::OnGamepadRemoved(winrt::Windows::Foundation::IInspectable const& sender, Gamepad const& args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_gamepad == args) {
        m_gamepad = nullptr;
        m_isConnected = false;
        // Find another gamepad if available
        auto gamepads = Gamepad::Gamepads();
        if (gamepads.Size() > 0) {
            m_gamepad = gamepads.GetAt(0);
            m_isConnected = true;
        }
    }
}

void ControllerManager::Update() {
    // Can be used for periodic check or state management
}

void ControllerManager::SetVibration(double leftTrigger, double rightTrigger, double leftMotor, double rightMotor) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isConnected && m_gamepad != nullptr) {
        GamepadVibration vibration;
        vibration.LeftTrigger = std::clamp(leftTrigger, 0.0, 1.0);
        vibration.RightTrigger = std::clamp(rightTrigger, 0.0, 1.0);
        vibration.LeftMotor = std::clamp(leftMotor, 0.0, 1.0);
        vibration.RightMotor = std::clamp(rightMotor, 0.0, 1.0);
        
        try {
            m_gamepad.Vibration(vibration);
        } catch (...) {
            // Handle disconnection or errors
            m_isConnected = false;
            m_gamepad = nullptr;
        }
    }
}
