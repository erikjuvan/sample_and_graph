#pragma once

#include "Device.hpp"
#include <thread>

class Acquisition
{
public:
    void GetData();
    bool ToggleConnect(); // return true if connected and false if disconnected
    void ConnectToDevices();
    void DisconnectFromDevices();
    bool ToggleStart();
    void StartDevices();
    void StopDevices();
    void Save();
    void Load(std::string const& fname);
    void Clear();

private:
    using AllTokens  = std::vector<std::vector<std::string>>;
    using LineTokens = std::vector<std::string>;

    // Methods
    AllTokens ParseConfigFile(const std::string& file_name);
    void      ConfigureFromTokens(AllTokens all_tokens);

    // Members
    std::vector<PhysicalDevice> m_physical_devices;
    std::vector<VirtualDevice>  m_virtual_devices;

    bool m_devices_connected{false};
    bool m_devices_running{false};

    std::thread m_thread_get_data;

    uint32_t m_sample_period_ms;
};
