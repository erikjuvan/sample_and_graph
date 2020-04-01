#include "MainWindow.hpp"
#include "Application.hpp"
#include "Communication.hpp"
#include "Helpers.hpp"
#include <fstream>
#include <functional>
#include <thread>

/*
void MainWindow::run()
{
    if (!*m_running) {
        *m_running = true;
        button_run->SetText("Running");
    } else {
        *m_running = false;
        button_save->Enabled(true);

        button_run->SetText("Stopped");
    }
}

void MainWindow::save()
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
        if (std::memcmp(&tmp, &header, sizeof(Header))) {
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
*/

MainWindow::MainWindow() :
    Window(1100, 600, "Sorting Control", sf::Style::None | sf::Style::Close)
{
    chart = std::make_shared<Chart>(100, 10, 990, 580, 100, 100);

    button_connect = std::make_shared<mygui::Button>(10, 10, "Connect");
    button_connect->OnClick([this] { signal_button_connect_Clicked(button_connect); });

    button_run = std::make_shared<mygui::Button>(10, 50, "Start");
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
