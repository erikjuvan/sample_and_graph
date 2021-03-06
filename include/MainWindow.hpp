#pragma once

#include <mygui/Action.hpp>
#include <mygui/Button.hpp>
#include <mygui/Checkbox.hpp>
#include <mygui/Textbox.hpp>

#include <lsignal.hpp>

#include "Chart.hpp"
#include "Device.hpp"
#include "Window.hpp"

class MainWindow : public Window
{
private:
    // Members
    const uint32_t m_Colors[10]{0xFF0000FF, 0x00FF00FF, 0x0000FFFF, 0xFFFF00FF, 0x00FFFFFF, 0xFF00FFFF, 0xFF8000FF, 0xC0C0C0FF, 0x800000FF, 0x808000FF};

    std::pair<bool, long long> m_run_start{false, 0};
    long long                  m_total_run_time{0};
    long long                  m_alive_start{0};

    // Widgets
    //////////

    std::shared_ptr<Chart> chart;

    // Using pyhsical devices
    std::shared_ptr<mygui::Button> button_connect; // connect/disconnects to/from physical devices, also unload any loaded data
    std::shared_ptr<mygui::Button> button_run;     // start capturing live data
    std::shared_ptr<mygui::Button> button_save;    // save captured data to file

    // Loading data from memory
    std::shared_ptr<mygui::Textbox> textbox_load;
    std::shared_ptr<mygui::Button>  button_load; // load data from external file, also stops and disconnects all devices

    // Used by both use cases
    std::shared_ptr<mygui::Button> button_clear; // clear all data

    // Select all / Deselect all checkboxes button
    std::shared_ptr<mygui::Button> button_sel_desel_all_chkbxs;

    // Checkboxes to enable/disable drawing of signals
    std::vector<std::shared_ptr<mygui::Checkbox>> checkboxes_signal_enabled;

    // Actions
    std::shared_ptr<mygui::Action> action_update_titlebar;

    // Widget callback functions
    void button_connect_clicked();
    void button_run_clicked();
    void button_load_clicked();
    void button_clear_clicked();

    // Private functions
    void UpdateTitleBar();

public:
    // Methods
    MainWindow();
    ~MainWindow();

    std::shared_ptr<Chart> Chart()
    {
        if (chart)
            return chart;
        else
            return nullptr;
    }

    lsignal::signal<bool()>                   signal_button_connect_Clicked;
    lsignal::signal<bool()>                   signal_button_run_Clicked;
    lsignal::signal<void()>                   signal_button_save_Clicked;
    lsignal::signal<void(std::string const&)> signal_button_load_Clicked;
    lsignal::signal<void()>                   signal_button_clear_Clicked;
};
