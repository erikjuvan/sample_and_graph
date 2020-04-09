#pragma once

#include "Device.hpp"
#include "lsignal.hpp"
#include <thread>

class Acquisition : public Serializer
{
public:
    // Signals
    lsignal::signal<void(std::vector<BaseDevice const*> const&)> signal_new_data;
    lsignal::signal<void(std::vector<BaseDevice const*> const&)> signal_devices_loaded;

    Acquisition() = default;
    ~Acquisition();

    virtual ser_data_t Serialize() const;
    virtual void       Deserialize(ser_data_t& data);

    bool     ToggleConnect(); // return true if connected and false if disconnected
    void     ConnectToDevices();
    void     DisconnectFromDevices();
    bool     ToggleStart();
    void     StartDevices();
    void     StopDevices();
    void     Save() const;
    void     Load(std::string const& fname);
    void     Clear();
    void     Reset();
    uint32_t GetSamplingPeriod() const;

private:
    using AllTokens  = std::vector<std::vector<std::string>>;
    using LineTokens = std::vector<std::string>;

    // Methods
    void      ReadData();
    AllTokens ParseConfigFile(const std::string& file_name);
    void      ConfigureFromTokens(AllTokens all_tokens);

    // Members
    std::vector<PhysicalDevice*> m_physical_devices;
    std::vector<VirtualDevice*>  m_virtual_devices;

    bool m_devices_connected{false};
    bool m_devices_running{false};

    std::thread m_thread_read_data;

    uint32_t m_sampling_period_ms{0};
};
