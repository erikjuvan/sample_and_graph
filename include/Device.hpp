#pragma once

#include "Communication.hpp"
#include "Serializer.hpp"
#include <memory>
#include <optional>

struct DataPacket {
    static constexpr uint32_t HEADER_START_ID = 0xDEADBEEF;

    static std::optional<DataPacket> Extract(const uint8_t* data, size_t size);
    static std::optional<DataPacket> Extract(std::vector<uint8_t> data) { return Extract(data.data(), data.size()); }

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

    virtual ser_data_t           Serialize() const override;
    virtual void                 Deserialize(ser_data_t& data) override;
    const std::vector<uint32_t>& GetBuffer() const { return m_buffer; }
    void                         AddToBuffer(uint32_t data) { m_buffer.push_back(data); }
    void                         AddToBuffer(std::vector<uint32_t> const& data) { m_buffer.insert(m_buffer.end(), data.begin(), data.end()); }
    void                         Name(std::string const& name) { m_name = name; }
    std::string                  Name() { return m_name; }
    void                         clear()
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
    virtual ser_data_t Serialize() const override;
    virtual void       Deserialize(ser_data_t& data) override;

    virtual void SetID(int id) { m_id = id; }
    virtual void SetName(std::string const& name) { m_name = name; }

    virtual void Clear()
    {
        for (auto& n : m_nodes)
            n.clear();
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
