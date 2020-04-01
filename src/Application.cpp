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

void Application::Save()
{
    // Check if there is anything to save
    // ...

    auto get_available_filename = [](std::string base_name) -> auto
    {
        std::string suffix;
        std::string extension{".txt"};
        int         cnt = 0;
        while (true) {
            std::string   fname = base_name + suffix + extension;
            std::ifstream f(fname);
            if (!f.good())
                return fname;
            suffix = "_" + std::to_string(++cnt);
        }
    };

    auto          fname = get_available_filename("temp_data");
    std::ofstream write_file(fname, std::ofstream::binary);

    if (write_file.is_open()) {

        // First... super purge
        // ...

        std::cout << "Saving data to " << fname << " ... ";

        // Write to file
        // ...

        write_file.close();
    } else {
        std::cerr << "Error: can't open file for writting!\n";
        return;
    }

    // Check file for correct header m_data
    /////////////////////////////////////
    std::ifstream read_file(fname, std::ifstream::binary);
    if (read_file.is_open()) {
        // Mem compare
        if (/*std::memcmp(&tmp, &header, sizeof(Header))*/ 0) {
            std::cerr << "Error write failed: Incorrect header when reading back file!\n";
            read_file.close();
            return;
        }
    } else {
        std::cerr << "Error: can't open file for reading!\n";
        return;
    }

    // Check file for correct size
    //////////////////////////////
    std::ifstream           in(fname, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type fsize = 0;
    if (in.is_open()) {
        fsize                                = in.tellg();
        std::ifstream::pos_type correct_size = 0; // TODO: get correct size
        in.close();
        if (fsize != correct_size) {
            std::cerr << "Error write failed: Written " << fsize << " bytes to file. Should have written " << correct_size << " bytes.\n";
            return;
        }
    } else {
        std::cerr << "Error: can't open file for reading!\n";
        return;
    }

    // All is well :)
    std::cout << "Successfully written " << fsize << " bytes to " << fname << std::endl;
}

void Application::GetData()
{
    std::future<void> future           = std::async(std::launch::async, [] { return; }); // create a valid future
    std::atomic_bool  parsing_too_slow = false;

    while (m_mainWindow->IsOpen()) {

        if (m_devices_running) {
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

void Application::StartDevices()
{
    if (m_devices_running)
        return;

    std::cout << "Started data acquisition\n\n";
    m_devices_running = true;
}

void Application::StopDevices()
{
    if (!m_devices_running)
        return;

    std::cout << "Stopped data acquisition\n\n";
    m_devices_running = false;
}

void Application::MainLoop()
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
    if (!m_devices_connected) {
        std::cout << "Connecting to devices...\n";
        // Initial parameters from file init
        auto tokens = ParseConfigFile("config.txt");
        ConfigureFromTokens(tokens);

        // Connect to configured devices
        bool connected = true;
        for (auto& dev : m_devices)
            connected &= dev.TryConnect(); // check if any connections fail by AND-ing

        if (!connected)
            throw std::runtime_error("Can't connect to all devices!");

        std::cout << "Connected to all devices\n\n";
        m_devices_connected = connected;
    }
}

void Application::DisconnectFromDevices()
{
    if (m_devices_connected) {
        m_devices_connected = false;
        m_devices.clear();
        std::cout << "Disconnected from all devices\n\n";
    }
}

Application::Application()
{
    // Set resource manager font name
    mygui::ResourceManager::SetSystemFontName("segoeui.ttf");

    // Create main window
    m_mainWindow = std::make_unique<MainWindow>();

    m_mainWindow->signal_button_connect_Clicked.connect([this]() {
        if (!m_devices_connected) {
            ConnectToDevices();
            return true;
        } else {
            StopDevices();
            DisconnectFromDevices();
            return false;
        }
    });

    m_mainWindow->signal_button_run_Clicked.connect([this]() {
        if (!m_devices_connected)
            return false;

        if (!m_devices_running) {
            StartDevices();
            return true;
        } else {
            StopDevices();
            return false;
        }
    });

    m_mainWindow->signal_button_save_Clicked.connect([this] {
        Save();
    });

    m_mainWindow->signal_button_load_Clicked.connect([this] {
        // Load data

        // If data load successfully disable "live capture" family of widgets (connect, run, save, ...)
    });

    m_mainWindow->signal_button_clear_data_Clicked.connect([this] {
        // Clear all data

        // Enable all widgets
    });
}

Application::~Application()
{
    if (m_thread_get_data.joinable())
        m_thread_get_data.join();
}
