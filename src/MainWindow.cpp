#include "MainWindow.hpp"

MainWindow::MainWindow() :
    Window(1100, 600, "Sample and Graph", sf::Style::None | sf::Style::Close)
{
    chart = std::make_shared<Chart>(100, 10, 990, 580, 100, 100);

    button_connect = std::make_shared<mygui::Button>(10, 10, "Connect");
    button_connect->OnClick([this] { signal_button_connect_Clicked(button_connect); });

    button_run = std::make_shared<mygui::Button>(10, 50, "Run");
    button_run->OnClick([this] { signal_button_run_Clicked(button_run); });

    button_save = std::make_shared<mygui::Button>(10, 90, "Save");
    button_save->OnClick([this] { signal_button_save_Clicked(); });

    button_load = std::make_shared<mygui::Button>(10, 160, "Load");
    button_load->OnClick([this] { signal_button_load_Clicked(); });

    button_clear_data = std::make_shared<mygui::Button>(10, 230, "Clear Data");
    button_clear_data->OnClick([this] { signal_button_clear_data_Clicked(); });

    // Add widgets

    Add(chart);

    Add(button_connect);
    Add(button_run);
    Add(button_save);

    Add(button_load);

    Add(button_clear_data);
}

MainWindow::~MainWindow()
{
}
