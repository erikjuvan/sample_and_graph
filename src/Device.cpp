#include "Device.hpp"
#include "Helpers.hpp"
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

Device::Device(std::string name, int id, std::vector<std::string> node_names)
{
    auto all_ports = m_serial_socket.ListPorts();

    // Extract only valid ports by checking description
    decltype(all_ports) ports;
    for (auto& it = all_ports.begin(); it != all_ports.end(); ++it)
        if (it->description.find("STMicroelectronics Virtual COM Port") != std::string::npos) // found it
            ports.push_back(*it);

    std::cout << "Initializing new Device\n";
    std::cout << "-----------------------\n";

    if (ports.empty()) {
        std::cout << "No available serial ports found!\n";
        std::cout << "-----------------------\n\n";
        return;
    }

    for (auto const& [p, desc, hw_id] : ports) {
        std::cout << "Trying " << p << "...\n";
        if (m_serial_socket.Connect(p)) {
            std::cout << "Connected to " << p << "\n";
            std::this_thread::sleep_for(1s); // Waiting to gather some data
            auto len = m_serial_socket.GetRxBufferLen();
            if (len < sizeof(DataPacket::HEADER_START_ID)) {
                std::cout << "No activity on " << p << ", Disconnecting!\n";
                m_serial_socket.Disconnect();
                continue;
            }
            std::vector<uint8_t> buf;
            auto                 read = m_serial_socket.Read(buf, len);
            std::cout << "Read " << read << " of available " << len << " bytes" << std::endl;
            std::cout << "Searching for valid data packet...\n";

            if (auto opt_packet = DataPacket::Extract(buf); opt_packet) {
                std::cout << "Valid data packet found\n";
                auto packet = *opt_packet;
                if (packet.header.device_id == id) { // Found ID
                    m_id   = id;
                    m_name = name;
                    for (auto const& name : node_names)
                        m_nodes.push_back(name);

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
            m_serial_socket.Disconnect();
    }

    if (!m_connected) {
        std::string msg = "Could not find device ID: " + std::to_string(m_id) + "(" + m_name + ")\n";
        std::cout << msg;
        std::cout << "-----------------------\n\n";
        throw std::exception(msg.c_str());
    } else {
        std::cout << "Device ID valid\nSuccessfully connected to device ID: " << m_id << "(" << m_name << ")\n";
        std::cout << "-----------------------\n\n";
    }
}

Device::~Device()
{
    if (m_serial_socket.IsConnected()) {
        m_serial_socket.Disconnect();
    }
}

std::vector<decltype(DataPacket::payload)> Device::GetData()
{
    if (m_connected) {
    }

    return std::vector<decltype(DataPacket::payload)>();
}
