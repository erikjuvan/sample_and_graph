#pragma once

#include "Communication.hpp"
#include "Serializer.hpp"
#include <memory>
#include <optional>

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
        uint32_t payload_size{0};
        uint32_t packet_id{0};
    };

    Header                header;
    std::vector<uint32_t> payload;
};

class Node : public Serializer
{
public:
    Node(std::string const& name) :
        m_name(name) {}
    Node() {}

    virtual ser_data_t Serialize() const override
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

    virtual void Deserialize(ser_data_t& data) override
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

    const std::vector<uint32_t>& GetBuffer() const
    {
        return m_buffer;
    }

    void AddToBuffer(uint32_t data)
    {
        m_buffer.push_back(data);
    }

    void AddToBuffer(std::vector<uint32_t> const& data)
    {
        m_buffer.insert(m_buffer.end(), data.begin(), data.end());
    }

    void Name(std::string const& name) { m_name = name; }

    std::string Name() { return m_name; }

    void clear()
    {
        m_name.clear();
        m_buffer.clear();
    }

private:
    std::string           m_name;
    std::vector<uint32_t> m_buffer;
};

class BaseDevice : public Serializer
{
public:
    virtual ser_data_t Serialize() const override
    {
        ser_data_t data;
        Serializer::append(data, "device");
        Serializer::append(data, m_id);
        Serializer::append(data, m_name, "\n");
        for (auto const& n : m_nodes)
            Serializer::append(data, n.Serialize());
        return data;
    }

    virtual void Deserialize(ser_data_t& data) override
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

protected:
    int               m_id{-1};
    std::string       m_name;
    std::vector<Node> m_nodes;
};

class VirtualDevice : public BaseDevice
{
public:
};

class PhysicalDevice : public BaseDevice
{
public:
    PhysicalDevice();
    ~PhysicalDevice();

    void SetName(std::string const& name) { m_name = name; }
    void SetID(int id) { m_id = id; }
    void SetNodes(std::vector<Node> const& nodes) { m_nodes = nodes; }
    void SetSamplePeriod(uint32_t period_ms);
    void Start();
    void Stop();
    bool TryConnect();
    void Disconnect();

private:
    std::shared_ptr<Communication> m_serial_socket;

    int  m_prev_packet_id{0};
    bool m_connected{false};
    bool m_running{false};
};
