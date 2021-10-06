#include "MainWindow.hpp"
#include <chrono>
#include <iomanip>

void MainWindow::button_connect_clicked()
{
    auto connected = signal_button_connect_Clicked();
    if (connected) {
        button_connect->SetText("Connected");
        button_connect->SetColor(sf::Color::Green);
        button_load->Enabled(false);
        m_total_run_time = 0;
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
        m_run_start.first  = true;
        m_run_start.second = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    } else {
        button_run->SetText("Run");
        button_run->ResetColor();
        button_save->Enabled(true);
        m_run_start.first = false;
        m_total_run_time += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - m_run_start.second;
    }
}

void MainWindow::button_load_clicked()
{
    signal_button_load_Clicked(textbox_load->GetText());
}

void MainWindow::button_clear_clicked()
{
    signal_button_clear_Clicked();
    m_total_run_time = 0;
}

void MainWindow::UpdateTitleBar()
{
    auto alive_msec                = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - m_alive_start;
    auto [running, run_start_time] = m_run_start;
    auto run_msec                  = run_start_time;
    if (running) {
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        run_msec = m_total_run_time + (now - run_start_time);
    } else
        run_msec = m_total_run_time;

    auto alive_sec = alive_msec / 1000;
    auto run_sec   = run_msec / 1000;
    //auto size = ;
    //auto capacity = ;
    // Update title bar
    std::stringstream str;
    str << "Sample and Graph    alive: " << std::to_string(alive_sec / 60) << ":" << std::setw(2) << std::setfill('0') << std::to_string(alive_sec % 60)
        << "  running: " << std::to_string(run_sec / 60) << ":" << std::setw(2) << std::setfill('0') << std::to_string(run_sec % 60); // << "   Buffer size: " << size << " MB" // Not implemented ATM
                                                                                                                                      // << " / " << capacity << " MB";         // Not implemented ATM
    SetTitle(str.str());
}

MainWindow::MainWindow() :
    Window(1230, 660, "Sample and Graph", sf::Style::None | sf::Style::Close)
{
    m_alive_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    chart = std::make_shared<::Chart>(100, 10, 1120, 640, 100, 2500);

    chart->signal_chart_signals_configured.connect([this](std::vector<std::shared_ptr<ChartSignal>> const& signals) {
        if (checkboxes_signal_enabled.size() > 0) {
            for (auto const& cb : checkboxes_signal_enabled)
                Remove(cb);

            checkboxes_signal_enabled.clear();
        }

        const int   spacing       = 23;
        auto const& lowest_button = button_sel_desel_all_chkbxs;
        const int   y_offset      = 15 + lowest_button->GetGlobalBounds().top + lowest_button->GetGlobalBounds().height;
        int         modulo        = (m_window->getSize().y - lowest_button->GetGlobalBounds().top -
                      lowest_button->GetGlobalBounds().height - spacing) /
                     spacing;
        for (int i = 0; i < signals.size(); ++i) {
            int column = i / modulo;
            int y      = y_offset + (i % modulo) * spacing;
            int x      = 10 + column * 45;

            checkboxes_signal_enabled.push_back(std::make_shared<mygui::Checkbox>(x, y, signals[i]->Name(), 13, 13, 13));
            checkboxes_signal_enabled.back()->Checked(true);
            Add(checkboxes_signal_enabled.back());
            checkboxes_signal_enabled.back()->OnClick([this, i] {
                chart->SetDrawChartSignal(i, checkboxes_signal_enabled.at(i)->Checked());
            });
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

    button_sel_desel_all_chkbxs = std::make_shared<mygui::Button>(10, 320, "(De)select");
    button_sel_desel_all_chkbxs->OnClick([this] {
        auto enabled = chart->ToggleDrawAllChartSignals();
        for (auto& cb : checkboxes_signal_enabled)
            cb->Checked(enabled);
    });

    action_update_titlebar = std::make_shared<mygui::Action>();
    action_update_titlebar->DoAction([this] { UpdateTitleBar(); });

    // Add widgets

    Add(chart);

    Add(button_connect);
    Add(button_run);
    Add(button_save);

    Add(textbox_load);
    Add(button_load);

    Add(button_clear);

    Add(button_sel_desel_all_chkbxs);

    Add(action_update_titlebar);
}

MainWindow::~MainWindow()
{
}
