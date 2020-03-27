#pragma once

#include <deque>

#include <mygui/Button.hpp>
#include <mygui/Checkbox.hpp>
#include <mygui/Label.hpp>
#include <mygui/Textbox.hpp>

#include "Application.hpp"
#include "Window.hpp"

class MainWindow : public Window
{
private:
    // Members
    const uint32_t m_Colors[10]{0xFF0000FF, 0x00FF00FF, 0x0000FFFF, 0xFFFF00FF, 0x00FFFFFF, 0xFF00FFFF, 0xFF8000FF, 0xC0C0C0FF, 0x800000FF, 0x808000FF};

    std::shared_ptr<bool> m_running;

    // Methods
    void RunClick();

    void button_connect_Click();
    void button_run_Click();
    void button_save_Click();

    void textbox_send_raw_EnterPress();

    void label_recv_raw_Clicked();

public:
    // Methods
    MainWindow(int w, int h, std::string const& title, sf::Uint32 style = sf::Style::Default);
    ~MainWindow();

    void ConnectCrossData(std::shared_ptr<bool> m_running);
    void UpdateWithNewData(const DataPacket& data);

    // Members
    //////////

    // Button
    std::shared_ptr<mygui::Button> button_connect;
    std::shared_ptr<mygui::Button> button_run;
    std::shared_ptr<mygui::Button> button_save;

    // Texbox
    std::shared_ptr<mygui::Textbox> textbox_send_raw;

    // Labels
    std::shared_ptr<mygui::Label> label_recv_raw;
};
