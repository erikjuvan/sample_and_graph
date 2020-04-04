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

void Application::MainLoop()
{
    while (m_mainWindow->IsOpen()) {
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

    m_mainWindow->signal_button_load_Clicked.connect([this] {
        // Load data

        // If data load successfully disable "live capture" family of widgets (connect, run, save, ...)
    });

    m_mainWindow->signal_button_clear_Clicked.connect([this] {
        m_acquisition->Clear();
    });
}

Application::~Application()
{
}
