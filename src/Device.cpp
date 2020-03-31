#include "Device.hpp"
#include "Helpers.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>

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

    // Kinda verbose and ugly but it works (find only free STM ports
    auto all_ports  = m_serial_socket->ListAllPorts();
    auto free_ports = m_serial_socket->ListFreePorts();
    for (auto& it = all_ports.begin(); it != all_ports.end();) {
        bool found = false;
        for (auto& fp = free_ports.begin(); fp != free_ports.end(); ++fp)
            if (it->port == *fp)
                found = true;

        if (!found)
            it = all_ports.erase(it);
        else
            ++it;
    }

    // Extract only valid ports by checking description
    decltype(all_ports) ports;
    for (auto& it = all_ports.begin(); it != all_ports.end(); ++it)
        if (it->description.find("STMicroelectronics Virtual COM Port") != std::string::npos) // found it
            ports.push_back(*it);

    auto dev_info = "ID:" + std::to_string(m_id) + " name:" + m_name;

    std::cout << "Initializing new Device (" << dev_info << ")\n";
    std::cout << "-----------------------\n";

    if (ports.empty()) {
        std::cout << "No available serial ports found!\n";
        std::cout << "-----------------------\n\n";
        return false;
    }

    for (auto const& [p, desc, hw_id] : ports) {
        std::cout << "Trying " << p << "...\n";
        if (m_serial_socket->Connect(p)) {
            std::cout << "Connected to " << p << "\n";
            std::this_thread::sleep_for(1s); // Waiting to gather some data
            auto len = m_serial_socket->GetRxBufferLen();
            if (len < sizeof(DataPacket::HEADER_START_ID)) {
                std::cout << "No activity on " << p << ", Disconnecting!\n";
                m_serial_socket->Disconnect();
                continue;
            }
            std::vector<uint8_t> buf;
            auto                 read = m_serial_socket->Read(buf, len);
            std::cout << "Read " << read << " of available " << len << " bytes" << std::endl;
            std::cout << "Searching for valid data packet...\n";

            if (auto opt_packet = DataPacket::Extract(buf); opt_packet) {
                std::cout << "Valid data packet found\n";
                auto packet = *opt_packet;
                if (packet.header.device_id == m_id) { // Found ID
                    m_connected = true;
                }
            } else {
                std::cout << "No valid data packet found!\n";
            }
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
        std::cout << "-----------------------\n\n";
        return false;
    } else {
        std::cout << "Device ID valid\nSuccessfully connected to device (" << dev_info << ")\n";
        std::cout << "-----------------------\n\n";
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
