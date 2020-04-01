#include "Device.hpp"
#include "Helpers.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

Device::Device()
{
    m_serial_socket = std::make_shared<Communication>();
}

bool Device::TryConnect()
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
            m_serial_socket->Write("ID_G");
            auto line = m_serial_socket->Readline();
            auto tok  = Help::TokenizeString(line, ",");
            if (tok.size() == 2 && tok[0] == "ID_G")
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

void Device::Disconnect()
{
    if (m_serial_socket->IsConnected()) {
        m_serial_socket->Disconnect();
    }
}

Device::~Device()
{
    Disconnect();
}

std::vector<decltype(DataPacket::payload)> Device::GetData() const
{
    if (m_connected) {
    }

    return std::vector<decltype(DataPacket::payload)>();
}
