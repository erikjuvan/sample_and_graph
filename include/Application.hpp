#pragma once

#include "Acquisition.hpp"
#include "MainWindow.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

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