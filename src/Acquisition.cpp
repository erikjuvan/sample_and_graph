#include "Acquisition.hpp"
#include "Helpers.hpp"
#include <algorithm>
#include <fstream>
#include <future>
#include <iostream>
#include <map>

Acquisition::~Acquisition()
{
    if (m_thread_read_data.joinable())
        m_thread_read_data.join();

    Clear();
}

Acquisition::AllTokens Acquisition::ParseConfigFile(const std::string& file_name)
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

void Acquisition::ConfigureFromTokens(Acquisition::AllTokens all_tokens)
{
    std::map<std::string, std::function<void(const LineTokens&)>> commands{
        {"sample_period", [this](const LineTokens& args) {
             auto prd = args.at(0);
             if (prd.find("ms") != std::string::npos)
                 m_sample_period_ms = std::stoi(args.at(0));
             else                                                   // seconds
                 m_sample_period_ms = std::stoi(args.at(0)) * 1000; // convert seconds to ms
         }},
        {"device", [this](const LineTokens& args) {m_physical_devices.push_back(new PhysicalDevice); if (args.size() > 0) m_physical_devices.back()->SetName(args.at(0)); }},
        {"id", [this](const LineTokens& args) { m_physical_devices.back()->SetID(std::stoi(args.at(0))); }},
        {"nodes", [this](const LineTokens& args) {
            for (auto arg : args)
                m_physical_devices.back()->push_back(Node(arg)); }},
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

void Acquisition::Save() const
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

    auto          fname = get_available_filename("data");
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

void Acquisition::Load(std::string const& fname)
{
    std::ifstream            in_file(fname, std::fstream::in);
    std::string              line;
    std::vector<std::string> lines;

    while (std::getline(in_file, line))
        lines.push_back(line);

    // Validate file data
    bool data_valid = false;
    // ...

    // If validated then clear existing data (devices)
    if (data_valid) {
        // Clear any existing devices
        Clear();
    } else {
        std::cout << "Input data invalid!\n";
        return;
    }

    // Load data
    std::cout << "Loading data '" << fname << "' ...\n";

    //...
    std::vector<BaseDevice const*> devices(m_virtual_devices.begin(), m_virtual_devices.end());
    signal_devices_loaded(devices);
}

void Acquisition::Clear()
{
    std::cout << "Acquisition::Clear\n";
    for (auto& d : m_physical_devices)
        delete d;
    for (auto& d : m_virtual_devices)
        delete d;
    m_physical_devices.clear();
    m_virtual_devices.clear();
}

void Acquisition::ReadData()
{
    while (m_devices_connected) {
        if (m_devices_running) {
            int cnt = 0;
            for (auto& dev : m_physical_devices)
                cnt += dev->ReadData();

            if (cnt > 0) {
                signal_new_data();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(
            m_sample_period_ms > 0 ? m_sample_period_ms : 100));
    }
}

bool Acquisition::ToggleStart()
{
    if (!m_devices_running)
        StartDevices();
    else
        StopDevices();

    return m_devices_running;
}

void Acquisition::StartDevices()
{
    if (!m_devices_connected)
        return;

    if (m_devices_running)
        return;

    for (auto& dev : m_physical_devices)
        dev->Start();

    std::cout << "Started data acquisition\n\n";
    m_devices_running = true;
}

void Acquisition::StopDevices()
{
    if (!m_devices_connected)
        return;

    if (!m_devices_running)
        return;

    for (auto& dev : m_physical_devices)
        dev->Stop();

    std::cout << "Stopped data acquisition\n\n";
    m_devices_running = false;
}

bool Acquisition::ToggleConnect()
{
    if (!m_devices_connected)
        ConnectToDevices();
    else
        DisconnectFromDevices();

    return m_devices_connected;
}

void Acquisition::ConnectToDevices()
{
    if (!m_devices_connected) {
        // First clear any existing device settings
        std::cout << "Clearing existing device settings...\n";
        Clear();

        std::cout << "Connecting to devices...\n";

        // Initial parameters from file init
        auto tokens = ParseConfigFile("config.txt");
        ConfigureFromTokens(tokens);

        // Connect to configured devices
        bool connected = true;
        for (auto& dev : m_physical_devices) {
            if (dev->TryConnect())
                dev->SetSamplePeriod(m_sample_period_ms);
            else
                connected = false;
        }

        if (!connected)
            throw std::runtime_error("Can't connect to all devices!");

        std::cout << "Connected to all devices\n\n";
        m_devices_connected = connected;
        StopDevices();
        std::vector<BaseDevice const*> devices(m_physical_devices.begin(), m_physical_devices.end());
        signal_devices_loaded(devices);
        m_thread_read_data = std::thread([this] { ReadData(); });
    }
}

void Acquisition::DisconnectFromDevices()
{
    if (m_devices_connected) {
        StopDevices();
        // Do not clear m_physical_devices, just manually disconnect. This allows saving data after disconnecting.
        for (auto& dev : m_physical_devices)
            dev->Disconnect();
        std::cout << "Disconnected from all devices\n\n";
        m_devices_connected = false;
        m_thread_read_data.join();
    }
}
