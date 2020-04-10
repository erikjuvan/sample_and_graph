#pragma once

#include "Communication.hpp"
#include "Serializer.hpp"
#include <memory>
#include <optional>

struct DataPacket {
    static constexpr uint32_t HEADER_START_ID = 0xDEADBEEF;

    static std::optional<DataPacket> Extract(const uint8_t* data, size_t size, int& remaining_size);
    static std::optional<DataPacket> Extract(std::vector<uint8_t>& data);

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
    ~Node() { clear(); }

    virtual ser_data_t           Serialize() const override;
    virtual void                 Deserialize(ser_data_t& data) override;
    const std::vector<uint32_t>& buffer() const { return m_buffer; }
    void                         push_back(uint32_t data) { m_buffer.push_back(data); }
    void                         append(std::vector<uint32_t> const& data) { m_buffer.insert(m_buffer.end(), data.begin(), data.end()); }
    void                         name(std::string const& name) { m_name = name; }
    std::string                  name() const { return m_name; }
    void                         clear() { m_buffer.clear(); }
    void                         reset()
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

    virtual void                     SetID(int id) { m_id = id; }
    virtual int                      GetID() const { return m_id; }
    virtual void                     SetName(std::string const& name) { m_name = name; }
    virtual std::string const&       GetName() const { return m_name; }
    virtual void                     push_back(Node const& node) { m_nodes.push_back(node); }
    virtual void                     AssignNodes(std::vector<Node> const& nodes) { m_nodes = nodes; }
    virtual std::vector<Node> const& GetNodes() const { return m_nodes; }
    virtual Node const&              GetNode(int idx) const { return m_nodes.at(idx); }
    virtual void                     Clear()
    {
        for (auto& n : m_nodes)
            n.clear();
    }
    virtual void Reset()
    {
        m_id = -1;
        m_name.clear();
        m_nodes.clear();
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

    void SetSamplingPeriod(uint32_t period_ms) const;
    void Start();
    void Stop();
    bool TryConnect();
    void Disconnect();
    int  ReadData();

private:
    std::shared_ptr<Communication> m_serial_socket;

    std::vector<uint8_t> m_raw_buffer;
    std::optional<int>   m_prev_packet_id;
    bool                 m_connected{false};
    bool                 m_running{false};
};
