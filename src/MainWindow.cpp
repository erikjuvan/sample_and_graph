#include "MainWindow.hpp"

void MainWindow::button_connect_clicked()
{
    auto connected = signal_button_connect_Clicked();
    if (connected) {
        button_connect->SetText("Connected");
        button_connect->SetColor(sf::Color::Green);
        button_load->Enabled(false);
    } else {
        button_connect->SetText("Connect");
        button_connect->ResetColor();
        button_run->SetText("Run");
        button_run->ResetColor();
        button_load->Enabled(true);
        button_save->Enabled(true);
    }
}

void MainWindow::button_run_clicked()
{
    auto running = signal_button_run_Clicked();
    if (running) {
        button_run->SetText("Running");
        button_run->SetColor(sf::Color::Green);
        button_save->Enabled(false);
    } else {
        button_run->SetText("Run");
        button_run->ResetColor();
        button_save->Enabled(true);
    }
}

void MainWindow::button_load_clicked()
{
    signal_button_load_Clicked(textbox_load->GetText());
}

void MainWindow::button_clear_clicked()
{
    signal_button_clear_Clicked();
}

MainWindow::MainWindow() :
    Window(1230, 600, "Sample and Graph", sf::Style::None | sf::Style::Close)
{
    chart = std::make_shared<::Chart>(100, 10, 1120, 580, 100, 100);

    chart->signal_chart_signals_configured.connect([this](std::vector<std::shared_ptr<ChartSignal>> const& signals) {
        if (checkboxes_signal_enabled.size() > 0) {
            for (auto const& cb : checkboxes_signal_enabled)
                Remove(cb);

            checkboxes_signal_enabled.clear();
        }

        const int spacing  = 23;
        const int y_offset = 15 + button_clear->GetGlobalBounds().top + button_clear->GetGlobalBounds().height;
        int       modulo   = (m_window->getSize().y - button_clear->GetGlobalBounds().top -
                      button_clear->GetGlobalBounds().height - spacing) /
                     spacing;
        for (int i = 0; i < signals.size(); ++i) {
            int column = i / modulo;
            int y      = y_offset + (i % modulo) * spacing;
            int x      = 10 + column * 45;

            checkboxes_signal_enabled.push_back(std::make_shared<mygui::Checkbox>(x, y, signals[i]->Name(), 13, 13, 13));
            checkboxes_signal_enabled.back()->Checked(true);
            Add(checkboxes_signal_enabled.back());
            checkboxes_signal_enabled.back()->OnClick([this, i] { chart->ToggleDrawChartSignal(i); });
        }
    });

    button_connect = std::make_shared<mygui::Button>(10, 10, "Connect");
    button_connect->OnClick([this] { button_connect_clicked(); });

    button_run = std::make_shared<mygui::Button>(10, 50, "Run");
    button_run->OnClick([this] { button_run_clicked(); });

    button_save = std::make_shared<mygui::Button>(10, 90, "Save");
    button_save->OnClick([this] { signal_button_save_Clicked(); });

    textbox_load = std::make_shared<mygui::Textbox>(10, 160, "data.txt");

    button_load = std::make_shared<mygui::Button>(10, 200, "Load");
    button_load->OnClick([this] { button_load_clicked(); });

    button_clear = std::make_shared<mygui::Button>(10, 270, "Clear Data");
    button_clear->OnClick([this] { button_clear_clicked(); });

    // Add widgets

    Add(chart);

    Add(button_connect);
    Add(button_run);
    Add(button_save);

    Add(textbox_load);
    Add(button_load);

    Add(button_clear);
}

MainWindow::~MainWindow()
{
}
