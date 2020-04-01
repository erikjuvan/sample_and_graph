#pragma once

#include <deque>

#include <mygui/Button.hpp>
#include <mygui/Checkbox.hpp>
#include <mygui/Label.hpp>
#include <mygui/Textbox.hpp>

#include <lsignal.hpp>

#include "Chart.hpp"
#include "Window.hpp"

class MainWindow : public Window
{
private:
    // Members
    const uint32_t m_Colors[10]{0xFF0000FF, 0x00FF00FF, 0x0000FFFF, 0xFFFF00FF, 0x00FFFFFF, 0xFF00FFFF, 0xFF8000FF, 0xC0C0C0FF, 0x800000FF, 0x808000FF};

    // Widgets
    //////////

    std::shared_ptr<Chart> chart;

    // Using pyhsical devices
    std::shared_ptr<mygui::Button> button_connect; // connect/disconnects to/from physical devices, also unload any loaded data
    std::shared_ptr<mygui::Button> button_run;     // start capturing live data
    std::shared_ptr<mygui::Button> button_save;    // save captured data to file

    // Loading data from memory
    std::shared_ptr<mygui::Button> button_load; // load data from external file, also stops and disconnects all devices

    // Used by both use cases
    std::shared_ptr<mygui::Button> button_clear_data; // clear all data

public:
    // Methods
    MainWindow();
    ~MainWindow();

    // Signals
    template <typename T>
    using Signal = lsignal::signal<T>;

    Signal<void(std::shared_ptr<mygui::Button>)> signal_button_connect_Clicked;
    Signal<void(std::shared_ptr<mygui::Button>)> signal_button_run_Clicked;
    Signal<void()>                               signal_button_save_Clicked;
    Signal<void()>                               signal_button_load_Clicked;
    Signal<void()>                               signal_button_clear_data_Clicked;
};
