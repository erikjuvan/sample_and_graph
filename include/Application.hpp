#pragma once

#include "Acquisition.hpp"
#include "MainWindow.hpp"

class Application
{
private:
    std::unique_ptr<MainWindow>  m_mainWindow;
    std::unique_ptr<Acquisition> m_acquisition;

public:
    Application();
    ~Application();

    void MainLoop();
};