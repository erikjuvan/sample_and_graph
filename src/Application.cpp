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

std::vector<std::vector<std::string>> Application::ParseConfigFile(const std::string& file_name)
{
    std::ifstream                         in_file(file_name);
    std::string                           str;
    std::vector<std::vector<std::string>> tokens;
    if (in_file.is_open()) {
        while (std::getline(in_file, str)) {

            // Remove everything after comment char '#'
            if (auto idx = str.find_first_of("#"); idx != std::string::npos)
                str = str.substr(0, idx);

            if (str.size() == 0)
                continue;

            tokens.push_back(Help::TokenizeString(str, " ,\t"));
        }
        in_file.close();
    }

    return tokens;
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
    auto tokens = ParseConfigFile("config.txt");

    std::vector<std::string> node_names{"1", "2"};
    m_devices.emplace_back("F407", 1, node_names);

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