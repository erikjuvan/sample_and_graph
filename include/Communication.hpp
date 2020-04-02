#pragma once

#include <mutex>
#include <serial/serial.h>

class Communication
{
public:
    Communication();
    ~Communication();

    // Making copy constructor a deleted function since serial has a deleted CC
    Communication(const Communication&) = delete;

    bool                          Connect(const std::string& port);
    void                          Disconnect();
    inline bool                   IsConnected() { return m_is_connected; }
    size_t                        GetRxBufferLen();
    size_t                        Write(const void* buffer, int size);
    size_t                        Write(const std::string& buffer);
    size_t                        Read(void* buffer, int size);
    size_t                        Read(std::vector<uint8_t>& buffer, size_t size = 1);
    std::string                   Readline();
    void                          Flush();
    void                          Purge();
    std::vector<serial::PortInfo> ListAllPorts();
    std::vector<std::string>      ListFreePorts();

    void                     SetTimeout(int ms);
    void                     ConfirmTransmission(std::string const& str); // throws on error
    std::vector<std::string> WriteAndTokenizeResult(std::string const& str);

private:
    std::mutex     m_mtx;
    serial::Serial m_serial;
    bool           m_is_connected{false};
};
