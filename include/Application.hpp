#pragma once

#include "Device.hpp"
#include "MainWindow.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

// Forward decleration since including headers with said classes causes Application.hpp to be included last and we get bunch of "variable undefined" errors
// TODO: check into it

class Application
{
private:
    using AllTokens  = std::vector<std::vector<std::string>>;
    using LineTokens = std::vector<std::string>;

    // Members
    std::unique_ptr<MainWindow> m_mainWindow;

    std::vector<PhysicalDevice> m_physical_devices;
    std::vector<VirtualDevice>  m_virtual_devices;

    bool m_devices_connected{false};
    bool m_devices_running{false};

    std::thread m_thread_get_data;

    uint32_t m_sample_period_ms;

    // Methods
    AllTokens ParseConfigFile(const std::string& file_name);
    void      ConfigureFromTokens(AllTokens all_tokens);
    void      GetData();

public:
    Application();
    ~Application();

    void ConnectToDevices();
    void DisconnectFromDevices();
    void StartDevices();
    void StopDevices();
    void MainLoop();
    void Save();
};