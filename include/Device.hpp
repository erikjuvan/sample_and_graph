#pragma once

#include "Communication.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct DataPacket {
    static constexpr uint32_t HEADER_START_ID = 0xDEADBEEF;

    static std::optional<DataPacket> Extract(const uint8_t* data, size_t size)
    {
        if (size < sizeof(HEADER_START_ID))
            return std::nullopt;

        for (int i = 0; i <= (size - sizeof(HEADER_START_ID)); ++i) {
            if (*((uint32_t*)&data[i]) == HEADER_START_ID) {
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

    void  AddToBuffer(uint32_t data) { buffer.push_back(data); }
    auto& GetBuffer() const { return buffer; }

private:
    std::string           m_name;
    std::vector<uint32_t> buffer;
};

class Device
{
public:
    Device();
    ~Device();

    void SetName(std::string const& name) { m_name = name; }
    void SetID(int id) { m_id = id; }
    void SetNodes(std::vector<Node> const& nodes) { m_nodes = nodes; }
    void SetSamplePeriod(uint32_t period_ms);
    void Start();
    void Stop();
    bool TryConnect();
    void Disconnect();

    std::vector<decltype(DataPacket::payload)> GetData() const;

private:
    std::shared_ptr<Communication> m_serial_socket;

    std::string       m_name{""};
    int               m_id{-1};
    std::vector<Node> m_nodes;
    int               m_prev_packet_id{0};
    bool              m_connected{false};
    bool              m_running{false};
};
