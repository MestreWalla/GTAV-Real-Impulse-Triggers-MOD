#pragma once
#define NOMINMAX

#include <winrt/Windows.Gaming.Input.h>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>

class ControllerManager {
public:
    ControllerManager();
    ~ControllerManager();

    void Update();
    void SetVibration(double leftTrigger, double rightTrigger, double leftMotor, double rightMotor);
    bool IsConnected() const { return m_isConnected; }

private:
    winrt::Windows::Gaming::Input::Gamepad m_gamepad = nullptr;
    std::mutex m_mutex;
    bool m_isConnected = false;

    void OnGamepadAdded(winrt::Windows::Foundation::IInspectable const &sender, winrt::Windows::Gaming::Input::Gamepad const &args);
    void OnGamepadRemoved(winrt::Windows::Foundation::IInspectable const &sender, winrt::Windows::Gaming::Input::Gamepad const &args);

    winrt::event_token m_addedToken;
    winrt::event_token m_removedToken;
};
