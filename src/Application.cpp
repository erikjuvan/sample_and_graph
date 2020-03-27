#include "Application.hpp"
#include "Communication.hpp"
#include "MainWindow.hpp"
#include <Helpers.hpp>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <mygui/ResourceManager.hpp>
#include <sstream>

using namespace std::chrono_literals;

Client::Client()
{
    auto ports = Help::GetCOMPorts();

    std::cout << "Initializing new Client\n";
    std::cout << "-----------------------\n";

    if (ports.empty()) {
        std::cout << "No available serial ports found!\n";
        std::cout << "-----------------------\n\n";
        return;
    }

    for (auto p : ports) {
        std::cout << "Trying " << p << "...\n";
        if (m_comm.Connect(p)) {
            std::cout << "Connected to " << p << "\n";
            std::this_thread::sleep_for(1s); // Waiting to gather some data
            auto len = m_comm.GetRxBufferLen();
            if (len < sizeof(DataPacket::Delim)) {
                std::cout << "No activity on " << p << ", Disconnecting!\n";
                m_comm.Disconnect();
                continue;
            }
            std::vector<uint8_t> buf;
            auto                 read = m_comm.Read(buf, len);
            std::cout << "Read " << read << " of available " << len << " bytes" << std::endl;
            std::cout << "Searching for valid data packet...\n";

            if (auto opt_packet = DataPacket::Extract(buf); opt_packet) {
                auto packet = *opt_packet;
                std::cout << "Valid data packet found\n";
                if (packet.header.sender_id >= 0) {
                    m_sender_id = packet.header.sender_id;
                    std::cout << "Sender ID valid\nSuccessfully connected to " << p << ", sender ID: " << *m_sender_id << std::endl;
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
            m_comm.Disconnect();
    }

    if (!m_connected)
        std::cout << "No available Senders found!\n";

    // Closing string
    std::cout << "-----------------------\n\n";
}

Client::~Client()
{
    if (m_comm.IsConnected()) {
        m_comm.Disconnect();
    }
}

void Application::GetData()
{
    std::future<void> future           = std::async(std::launch::async, [] { return; }); // create a valid future
    std::atomic_bool  parsing_too_slow = false;

    while (m_mainWindow->IsOpen()) {

        if (*m_running) {
            /*
            ClientData data;
            for (auto const& c : clients)
            {
                data += c.GetData();
            }
            */
        }
    }
}

void Application::InitFromFile(const std::string& file_name)
{
    std::ifstream            in_file(file_name);
    std::string              str;
    std::vector<std::string> tokens;
    if (in_file.is_open()) {
        while (std::getline(in_file, str)) {
            tokens.push_back(str);
        }
        in_file.close();
    }

    for (int i = 0; i < tokens.size(); ++i) {
        switch (i) {
        case 0: // Line 1
            break;
        case 1: // Line 2
            break;
        }
    }
}

void Application::Run()
{
    // Create threads that are the brains of the program
    m_thread_get_data = std::thread(std::bind(&Application::GetData, this));

    while (m_mainWindow->IsOpen()) {
        m_mainWindow->Update();
        // 60 FPS is enough
        std::this_thread::sleep_for(15ms);
    }
}

Application::Application()
{
    // Initial parameters from file init
    InitFromFile("config.txt");

    // Set resource manager font name
    mygui::ResourceManager::SetSystemFontName("segoeui.ttf");

    // Set state variables
    m_running = std::make_shared<bool>(false);

    // Create main window
    m_mainWindow = std::make_unique<MainWindow>(900, 500, "Sorting Control", sf::Style::None | sf::Style::Close);

    // Now that all objects are created pass all neccessary data to mainwindow
    m_mainWindow->ConnectCrossData(m_running);
}

Application::~Application()
{
    m_thread_get_data.join();
}