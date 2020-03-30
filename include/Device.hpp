#pragma once

#include "Communication.hpp"
#include <optional>
#include <string>
#include <vector>

struct DataPacket {
    static constexpr uint32_t Delim = 0xDEADBEEF;

    static std::optional<DataPacket> Extract(const uint8_t* data, size_t size)
    {
        if (size < sizeof(Delim))
            return std::nullopt;

        for (int i = 0; i <= (size - sizeof(Delim)); ++i)
            if (*((uint32_t*)&data[i]) == Delim) {
                DataPacket data_packet;
                int        remaining_size = size - i;
                if (remaining_size < sizeof(Header))
                    return std::nullopt; // invalid header
                // Copy header
                memcpy(&data_packet.header, &data[i], sizeof(Header));
                int payload_idx = i + sizeof(Header);
                remaining_size  = size - payload_idx;
                if (remaining_size < data_packet.header.payload_size)
                    return std::nullopt; // invalid payload
                uint32_t* payload_data = (uint32_t*)&data[payload_idx];
                int       payload_len  = data_packet.header.payload_size / sizeof(decltype(DataPacket::payload)::value_type);
                data_packet.payload.assign(&payload_data[0], &payload_data[payload_len]);
                return data_packet;
            }

        return std::nullopt;
    }

    static std::optional<DataPacket> Extract(std::vector<uint8_t> data)
    {
        return Extract(data.data(), data.size());
    }

    struct Header {
        uint32_t header_start_id{0};
        int8_t   device_id{-1};
        uint32_t payload_size{0};
        uint32_t packet_id{0};
    };

    Header                header;
    std::vector<uint32_t> payload;
};

class Node
{
public:
    Node(std::string name) :
        m_name(name) {}

    void AddToBuffer(uint32_t data) { buffer.push_back(data); }
    auto GetBuffer() { return buffer; }

private:
    std::string           m_name;
    std::vector<uint32_t> buffer;
};

class Device
{
public:
    Device::Device(std::string name, int id, std::vector<std::string> node_names);
    Device::~Device();

    std::vector<decltype(DataPacket::payload)> GetData();

private:
    Communication m_serial_socket;

    std::string       m_name{""};
    int               m_id{-1};
    std::vector<Node> m_nodes;
    int               m_prev_packet_id{0};
    bool              m_connected = false;
};
