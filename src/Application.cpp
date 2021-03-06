#include "Application.hpp"
#include <mygui/ResourceManager.hpp>

using namespace std::chrono_literals;

void Application::MainLoop()
{
    while (m_mainWindow->IsOpen()) {
        m_acquisition->ReadData();
        m_mainWindow->Update();
        // 60 FPS is enough
        std::this_thread::sleep_for(15ms);
    }
}

Application::Application()
{
    // Set resource manager font name
    mygui::ResourceManager::SetSystemFontName("segoeui.ttf");

    // Create main window
    m_mainWindow = std::make_unique<MainWindow>();

    // Create new acquisition module
    m_acquisition = std::make_unique<Acquisition>();

    m_mainWindow->signal_button_connect_Clicked.connect([this]() {
        return m_acquisition->ToggleConnect();
    });

    m_mainWindow->signal_button_run_Clicked.connect([this]() {
        return m_acquisition->ToggleStart();
    });

    m_mainWindow->signal_button_save_Clicked.connect([this] {
        m_acquisition->Save();
    });

    m_mainWindow->signal_button_load_Clicked.connect([this](std::string const& fname) {
        m_acquisition->Load(fname);
    });

    m_mainWindow->signal_button_clear_Clicked.connect([this] {
        m_mainWindow->Chart()->ClearChartSignals();
        m_acquisition->Clear();
        m_mainWindow->Chart()->SetAxisX(0);
    });

    m_acquisition->signal_new_data.connect([this](std::vector<BaseDevice const*> const& devices) {
        m_mainWindow->Chart()->Update(devices);
    });

    m_acquisition->signal_devices_loaded.connect([this](std::vector<BaseDevice const*> const& devices) {
        m_mainWindow->Chart()->SetSamplingPeriod(m_acquisition->GetSamplingPeriod());
        m_mainWindow->Chart()->LoadDevices(devices);
    });
}

Application::~Application()
{
}
