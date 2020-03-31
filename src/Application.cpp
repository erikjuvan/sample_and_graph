#include "Application.hpp"
#include "Communication.hpp"
#include "MainWindow.hpp"
#include <Helpers.hpp>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
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

Application::AllTokens Application::ParseConfigFile(const std::string& file_name)
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

void Application::ConfigureFromTokens(Application::AllTokens all_tokens)
{
    std::map<std::string, std::function<void(const LineTokens&)>> commands{
        {"sample_period", [this](const LineTokens& args) { m_sampling_period = std::stoi(args.at(0)); }},
        {"device", [this](const LineTokens& args) {m_devices.emplace_back(); if (args.size() > 0) m_devices.back().SetName(args.at(0)); }},
        {"id", [this](const LineTokens& args) { m_devices.back().SetID(std::stoi(args.at(0))); }},
        {"nodes", [this](const LineTokens& args) { m_devices.back().SetNodes(std::vector<Node>(args.begin(), args.end())); }},
    };

    for (auto line_tokens : all_tokens) {
        auto& cmd = line_tokens[0];
        std::transform(cmd.begin(), cmd.end(), cmd.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        auto args = LineTokens(line_tokens.begin() + 1, line_tokens.end());
        // Run command
        try {
            commands.at(cmd)(args);
        } catch (std::out_of_range) {
            std::cout << "Error trying to configure device. Command '" << cmd << "' unknown!\n";
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

void Application::ConnectToDevices()
{
    // Initial parameters from file init
    auto tokens = ParseConfigFile("config.txt");
    ConfigureFromTokens(tokens);

    // Connect to configured devices
    m_connected = true;
    for (auto& dev : m_devices)
        m_connected &= dev.TryConnect(); // check if any connections fail by AND-ing

    if (!m_connected)
        throw std::runtime_error("Can't find all devices!");
}

Application::Application()
{
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
    if (m_thread_get_data.joinable())
        m_thread_get_data.join();
}