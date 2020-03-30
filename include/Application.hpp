#pragma once

#include "Communication.hpp"
#include "Device.hpp"
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
class MainWindow;

class Application
{
private:
    // Members
    std::unique_ptr<MainWindow> m_mainWindow;

    std::deque<Device> m_devices;

    std::shared_ptr<bool> m_running;

    std::thread m_thread_get_data;

    uint32_t m_sampling_period;

    // Methods
    std::vector<std::vector<std::string>> ParseConfigFile(const std::string& file_name);
    void                                  GetData();

public:
    Application();
    ~Application();

    void Run();
};