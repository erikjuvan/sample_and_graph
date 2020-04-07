#include "Device.hpp"
#include "Helpers.hpp"
#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

std::optional<DataPacket> DataPacket::Extract(const uint8_t* data, size_t size, int& remaining_size)
{
    if (size <= sizeof(Header)) // only header, we also need data
        return std::nullopt;

    for (int i = 0; i <= (size - sizeof(HEADER_START_ID)); ++i) {
        if (*((uint32_t*)&data[i]) == HEADER_START_ID) {
            DataPacket data_packet;
            int        rem_size = size - i;
            if (rem_size < sizeof(Header))
                return std::nullopt; // invalid header
            // Copy header
            memcpy(&data_packet.header, &data[i], sizeof(Header));
            int payload_idx = i + sizeof(Header);
            rem_size        = size - payload_idx;
            if (rem_size < data_packet.header.payload_size)
                return std::nullopt; // invalid payload
            uint32_t* payload_data = (uint32_t*)&data[payload_idx];
            int       payload_len  = data_packet.header.payload_size / sizeof(decltype(DataPacket::payload)::value_type);
            data_packet.payload.assign(&payload_data[0], &payload_data[payload_len]);
            rem_size -= data_packet.header.payload_size;
            remaining_size = rem_size;
            return data_packet;
        }
    }

    return std::nullopt;
}

std::optional<DataPacket> DataPacket::Extract(std::vector<uint8_t>& data)
{
    if (data.size() <= 0)
        return std::nullopt;

    int  remaining_size = 0;
    auto ret            = Extract(data.data(), data.size(), remaining_size);
    data.erase(data.begin(), data.begin() + (data.size() - remaining_size));
    return ret;
}

Serializer::ser_data_t Node::Serialize() const
{
    ser_data_t data;
    Serializer::append(data, "node");
    Serializer::append(data, m_name);
    for (auto x : m_buffer)
        Serializer::append(data, x);
    data.pop_back();
    Serializer::append(data, "\n", "");
    return data;
}

void Node::Deserialize(ser_data_t& data)
{
    auto newline_it = std::find(data.begin(), data.end(), '\n');

    std::string              str(data.begin(), newline_it);
    std::istringstream       ss(str);
    std::string              str_tok;
    std::vector<std::string> tokens;

    while (std::getline(ss, str_tok, Serializer::Delim[0]))
        tokens.push_back(str_tok);

    if (tokens.size() > 2 && tokens[0] == "node") {
        m_name = tokens[1];
        m_buffer.clear();
        auto beg = tokens.begin() + 2;
        for (auto it = beg; it != tokens.end(); ++it)
            m_buffer.push_back(std::stoi(*it));
    }

    data = ser_data_t(newline_it + 1 /* skip newline */, data.end());
}

Serializer::ser_data_t BaseDevice::Serialize() const
{
    ser_data_t data;
    Serializer::append(data, "device");
    Serializer::append(data, m_id);
    Serializer::append(data, m_name, "\n");
    for (auto const& n : m_nodes)
        Serializer::append(data, n.Serialize());
    return data;
}

void BaseDevice::Deserialize(ser_data_t& data)
{
    auto newline_it = std::find(data.begin(), data.end(), '\n');

    std::string              str(data.begin(), newline_it);
    std::istringstream       ss(str);
    std::string              str_tok;
    std::vector<std::string> tokens;

    while (std::getline(ss, str_tok, Serializer::Delim[0]))
        tokens.push_back(str_tok);

    if (tokens.size() == 3 && tokens[0] == "device") {
        m_id   = std::stoi(tokens[1]);
        m_name = tokens[2];
    }

    data = ser_data_t(newline_it + 1 /* skip newline */, data.end());
    // Check if entry is for a node or a new device
    while (std::string(data.begin(), std::find(data.begin(), data.end(), ',')) == "node")
        m_nodes.emplace_back().Deserialize(data);
}

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
    if (m_running)
        Stop();

    if (m_serial_socket->IsConnected()) {
        m_serial_socket->Disconnect();
    }
    m_connected = false;
}

PhysicalDevice::~PhysicalDevice()
{
    Disconnect();
}

void PhysicalDevice::SetSamplePeriod(uint32_t period_ms) const
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
    m_running = true;
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

    // Reset m_prev_packet_id since we lost some while stopping device
    m_prev_packet_id = std::nullopt;

    m_running = false;
}

// Read data from serial port
int PhysicalDevice::ReadData()
{
    if (!m_running)
        return 0;

    int cnt = 0;

    if (auto size = m_serial_socket->GetRxBufferLen(); size > 0) {
        decltype(m_raw_buffer) vec;
        m_serial_socket->Read(vec, size);
        m_raw_buffer.insert(m_raw_buffer.end(), vec.begin(), vec.end());
        for (auto dp = DataPacket::Extract(m_raw_buffer); dp; dp = DataPacket::Extract(m_raw_buffer)) {
            if (dp->payload.size() != m_nodes.size())
                throw std::length_error("Payload length " + std::to_string(dp->payload.size()) +
                                        " is not equal to nodes size " + std::to_string(m_nodes.size()));

            if (m_prev_packet_id && dp->header.packet_id != *m_prev_packet_id + 1)
                std::cout << "Missed packet! Expected packet id:" << *m_prev_packet_id + 1 << " received id:" << dp->header.packet_id << "\n";

            m_prev_packet_id = dp->header.packet_id;

            for (int i = 0; i < dp->payload.size(); ++i)
                m_nodes[i].push_back(dp->payload[i]);

            cnt++;
        }
    }

    return cnt;
}
