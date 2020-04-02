#include "Device.hpp"
#include "Helpers.hpp"
#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

PhysicalDevice::PhysicalDevice()
{
    m_serial_socket = std::make_shared<Communication>();
}

bool PhysicalDevice::TryConnect()
{
    // Check if device is configured
    if (m_id < 0 && m_nodes.size() == 0) {
        std::cout << "Device not configured, please configure device before trying to connect.\n";
        return false;
    }

    // Find only free ports
    auto all_ports  = m_serial_socket->ListAllPorts();
    auto free_ports = m_serial_socket->ListFreePorts();
    for (auto it = all_ports.begin(); it != all_ports.end();) {
        bool found = false;
        for (auto fp = free_ports.begin(); fp != free_ports.end(); ++fp)
            if (it->port == *fp)
                found = true;

        if (!found)
            it = all_ports.erase(it);
        else
            ++it;
    }

    // Extract ports of valid STM32 devices by checking description
    decltype(all_ports) ports;
    for (auto it = all_ports.begin(); it != all_ports.end(); ++it)
        if (it->description.find("STMicroelectronics Virtual COM Port") != std::string::npos) // found it
            ports.push_back(*it);

    auto dev_info = "ID:" + std::to_string(m_id) + " name:" + m_name;

    std::cout << "Initializing new Device (" << dev_info << ")\n";
    std::cout << "-----------------------\n";

    if (ports.empty()) {
        std::cout << "No available serial ports found!\n";
        std::cout << "-----------------------\n";
        return false;
    }

    for (auto const& [p, desc, hw_id] : ports) {
        std::cout << "Trying " << p << "...\n";
        if (m_serial_socket->Connect(p)) {
            std::cout << "Connected to " << p << "\n";
            // Make sure device is stopped
            Stop();
            auto tok = m_serial_socket->WriteAndTokenizeResult("ID_G\n");
            if (tok.size() == 2 && tok.at(0) == "ID_G")
                if (auto id = std::stoi(tok[1]); id == m_id)
                    m_connected = true;

        } else {
            std::cout << "Can't connect to " << p << "!\n";
        }

        if (m_connected)
            break;
        else
            m_serial_socket->Disconnect();
    }

    if (!m_connected) {
        std::cout << "Could not find device (" << dev_info << ")\n";
        std::cout << "-----------------------\n";
        return false;
    } else {
        std::cout << "Successfully connected to device (" << dev_info << ")\n";
        std::cout << "-----------------------\n";
    }

    return true;
}

void PhysicalDevice::Disconnect()
{
    if (m_serial_socket->IsConnected()) {
        m_serial_socket->Disconnect();
    }
}

PhysicalDevice::~PhysicalDevice()
{
    Disconnect();
}

void PhysicalDevice::SetSamplePeriod(uint32_t period_ms)
{
    auto cmd = "PRDS," + std::to_string(period_ms) + "\n";
    m_serial_socket->Write(cmd);
    m_serial_socket->ConfirmTransmission(cmd);
}

void PhysicalDevice::Start()
{
    auto cmd = "STRT\n";
    m_serial_socket->Write(cmd);
    m_serial_socket->ConfirmTransmission(cmd);
}

void PhysicalDevice::Stop()
{
    using namespace std::chrono_literals;
    auto cmd = "STOP\n";

    // Stop transmission
    m_serial_socket->Write(cmd);

    // Wait for echo from upper write
    std::promise<void> prom;
    auto               fut = prom.get_future();
    std::thread        thr([this, &prom] {while (m_serial_socket->GetRxBufferLen() <= 0); prom.set_value(); });

    if (fut.wait_for(1s) != std::future_status::ready) {
        auto        dev_info = "ID:" + std::to_string(m_id) + " name:" + m_name;
        std::string msg("Can't stop device " + dev_info);
        throw std::runtime_error(msg.c_str());
    }

    thr.join();

    // Clear data while it's there
    for (auto data_in_buf = m_serial_socket->GetRxBufferLen(); data_in_buf > 0; data_in_buf = m_serial_socket->GetRxBufferLen()) {
        m_serial_socket->Purge();
        m_serial_socket->Flush();
        std::this_thread::sleep_for(10ms);
    }

    // Make sure we are really stopped by sending the command and checking for confirmation
    m_serial_socket->Write(cmd);
    m_serial_socket->ConfirmTransmission(cmd);
}
