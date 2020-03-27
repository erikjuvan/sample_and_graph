#pragma once

#include <Communication.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

constexpr int N_CLIENTS{3};
constexpr int N_CHANNELS{8};
constexpr int SAMPLE_FREQ = 1; // 1 Hz

// Forward decleration since including headers with said classes causes Application.hpp to be included last and we get bunch of "variable undefined" errors
// TODO: check into it
class MainWindow;

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
                memcpy(&data_packet, &data[i], remaining_size <= sizeof(data_packet) ? remaining_size : sizeof(data_packet));
                return data_packet;
            }

        return std::nullopt;
    }

    static std::optional<DataPacket> Extract(std::vector<uint8_t> data)
    {
        return Extract(data.data(), data.size());
    }

    struct Header {
        uint32_t delim{0};
        int8_t   sender_id{-1};
        uint32_t packet_id = 0;
    };

    Header   header;
    uint32_t buf[N_CHANNELS]{0};
};

class Client
{
public:
    Client();
    ~Client();

    std::vector<std::array<uint32_t, N_CHANNELS>> GetData()
    {
        if (m_connected) {
        }
    }

private:
    Communication m_comm;

    int                m_prev_packet_id{0};
    std::optional<int> m_sender_id;
    bool               m_connected = false;
};

class Application
{
private:
    // Members
    std::unique_ptr<MainWindow> m_mainWindow;

    std::array<Client, N_CLIENTS> clients;

    std::shared_ptr<bool> m_running;

    std::thread m_thread_get_data;

    // Methods
    void InitFromFile(const std::string& file_name);
    void GetData();

public:
    Application();
    ~Application();

    void Run();
};