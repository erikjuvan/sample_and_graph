#include "Chart.hpp"
#include <iomanip>
#include <mygui/ResourceManager.hpp>
#include <sstream>

Chart::Chart(int x, int y, int w, int h, int num_of_points, float max_val) :
    m_num_of_points(num_of_points), m_max_val(max_val), m_background(sf::Vector2f(w, h)),
    m_chart_region(sf::Vector2f(w - 6 * m_margin, h - 5 * m_margin))
{
    m_background.setPosition(x, y);
    m_background.setOutlineColor(sf::Color::Black);
    m_background.setOutlineThickness(1.f);

    m_chart_region.setPosition(x + 4.f * m_margin, y + 2.f * m_margin);
    m_chart_region.setOutlineColor(sf::Color::Black);
    // We need to get global bounds before setting outline thickness, because it adds the thickness to width and height
    // (outline increases dimensions it doesn't take from it)
    m_chart_rect = m_chart_region.getGlobalBounds();
    m_chart_region.setOutlineThickness(1.f);

    m_font.loadFromFile(mygui::ResourceManager::GetSystemFontName());
    m_title.setFont(m_font);
    m_title.setFillColor(sf::Color::Black);
    m_title.setString("Sorting control");
    m_title.setPosition(sf::Vector2f(x + w / 2.f - m_title.getLocalBounds().width / 2.f, y));

    m_x_axis.setFont(m_font);
    m_x_axis.setFillColor(sf::Color::Black);
    m_x_axis.setCharacterSize(24);
    m_x_axis.setString("Time / min");
    m_x_axis.setPosition(sf::Vector2f(x + w / 2.f - m_x_axis.getLocalBounds().width / 2.f, h - 1.25f * m_margin));

    m_y_axis.setFont(m_font);
    m_y_axis.setFillColor(sf::Color::Black);
    m_y_axis.setCharacterSize(24);
    m_y_axis.setRotation(-90.f);
    m_y_axis.setString("Temperature / *C");
    m_y_axis.setPosition(sf::Vector2f(x + m_margin / 4.f, y + h / 2.f + m_y_axis.getLocalBounds().width / 2.f));

    CreateGrid(11, 9);
}

void Chart::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_background, states);
    target.draw(m_chart_region, states);
    target.draw(m_x_axis);
    target.draw(m_y_axis);
    //target.draw(m_title);
    target.draw(m_grid);
    for (auto& m : m_x_axis_markers)
        target.draw(m);
    for (auto& m : m_y_axis_markers)
        target.draw(m);
    for (int i = 0; i < m_chart_signals.size(); ++i) {
        target.draw(*m_chart_signals[i]);
    }
}

void Chart::Handle(const sf::Event& event)
{
    if (!Enabled())
        return;

    auto change_draw_index = [this](int ci) {
        if (m_chart_signals.size() <= 0)
            return;

        for (auto& ch : m_chart_signals)
            ch->ChangeDrawIndex(ci);

        SetAxisX(m_chart_signals.front()->GetDrawIndex());
    };

    //  && m_chart_region.getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y))
    if (event.type == sf::Event::MouseWheelScrolled && m_mouseover) {
        // Currently not implemented
        //if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
        //    if (m_max_val >= 100.f) {
        //        m_max_val -= event.mouseWheelScroll.delta * 50.f;
        //    } else if (m_max_val > 5.f) {
        //        m_max_val -= event.mouseWheelScroll.delta * 5.f;
        //    } else {
        //        if (event.mouseWheelScroll.delta < 0.f)
        //            m_max_val -= event.mouseWheelScroll.delta * 5.f;
        //    }

        //    CreateAxisMarkers();
        //}
    } else if (event.type == sf::Event::KeyReleased && m_mouseover) {
        if (m_onKeyPress)
            m_onKeyPress(event);
    } else if (m_mouseover && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        m_mouse_drag_start_pos_x    = event.mouseButton.x;
        m_holding_left_mouse_button = true;
    } else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        m_holding_left_mouse_button = false;
        if (m_mouseover)
            change_draw_index(m_mouse_drag_start_pos_x - event.mouseButton.x);
    } else if (event.type == sf::Event::MouseMoved) {
        if (m_chart_region.getGlobalBounds().contains(sf::Vector2f(event.mouseMove.x, event.mouseMove.y))) {
            m_mouseover = true;
            if (m_holding_left_mouse_button) {
                change_draw_index(m_mouse_drag_start_pos_x - event.mouseMove.x);

                m_mouse_drag_start_pos_x = event.mouseMove.x;
            }

        } else {
            m_mouseover = false;
        }
    }
}

void Chart::Enabled(bool enabled)
{
    m_enabled = enabled;
    if (enabled) {
    } else {
    }
}

bool Chart::Enabled() const
{
    return m_enabled;
}

// Convert data from raw ADC voltage to NTC temperature
void Chart::ConvertData(std::vector<float>& data)
{
    // Calculate degrees from raw data
    // NTC equation
    const float vcc   = 3.0;
    const float r1    = 5600.f;
    const float beta  = 4920.f; // datasheet
    const float rntc0 = 33000.f;
    const float t0    = 298.15; // 25 *C
    const float rinf  = rntc0 * std::exp(-beta / t0);
    for (auto& y : data) {
        //auto vntc = vcc * y / 4096.f;
        // Vntc = Vcc * Rntc / (R1 + Rntc);
        // Vntc * R1 + Vntc * Rntc = Vcc * Rntc
        // Rntc * (Vcc - Vntc) = Vntc * R1
        // Rntc = Vntc / (Vcc - Vntc) * R1;
        //float rntc = vntc / (vcc - vntc) * r1;

        // Temperature: Tntc = beta / (ln(Rntc/Rinf))
        //y = beta / std::log(rntc / rinf) - 273.15;

        // Using Wolfram alpha I simplified the equation. Notice Vcc is not used, since to calculate rntc we only need the adc ratio value, no the absolute.
        y = beta / (std::log(-y / (y - 4096.f)) + 14.728) - 273.15;
    }
}

void Chart::LoadDevices(std::vector<BaseDevice const*> const& devices)
{
    m_chart_signals.clear();

    for (auto& d : devices) {
        for (auto& n : d->GetNodes()) {
            m_chart_signals.push_back(std::make_shared<ChartSignal>(m_chart_rect));
            m_chart_signals.back()->Name(n.name());
            if (n.buffer().size() > 0) {
                std::vector<float> buf(n.buffer().begin(), n.buffer().end());
                ConvertData(buf);
                m_chart_signals.back()->Append(buf);
            }
        }
    }
    CreateAxisMarkers();
    SetAxisX(m_chart_signals.front()->GetDrawIndex());
    signal_chart_signals_configured(m_chart_signals);
}

void Chart::Update(std::vector<BaseDevice const*> const& devices)
{
    auto it = m_chart_signals.begin();
    for (auto& d : devices) {
        for (auto& n : d->GetNodes()) {
            auto vec = std::vector<float>(n.buffer().end() - (n.buffer().size() - (*it)->Data().size()), n.buffer().end());

            ConvertData(vec);

            (*it)->Append(vec);
            it++;
        }
    }

    SetAxisX(m_chart_signals.front()->GetDrawIndex());
}

void Chart::AddChartSignal(std::shared_ptr<ChartSignal> const& csignal)
{
    m_chart_signals.push_back(csignal);
}

void Chart::ChangeChartSignal(int idx, std::shared_ptr<ChartSignal> const& csignal)
{
    if (idx < m_chart_signals.size()) {
        m_chart_signals[idx] = csignal;
    }
}

// n_lines - number of one type of lines (vertical or horizontal), there are same number of other lines
void Chart::CreateGrid(int n_lines_x, int n_lines_y)
{
    m_num_grid_lines_x    = n_lines_x;
    m_num_grid_lines_y    = n_lines_y;
    m_grid                = sf::VertexArray(sf::PrimitiveType::Lines, n_lines_x * 2 + n_lines_y * 2);
    const sf::Color color = sf::Color(100, 100, 100, 65);

    // vertical lines
    for (int i = 0; i < n_lines_x; ++i) {
        m_grid[2 * i].position     = sf::Vector2f(m_chart_rect.left + (i + 1) * (m_chart_rect.width / (n_lines_x + 1)), m_chart_rect.top);
        m_grid[2 * i].color        = color;
        m_grid[2 * i + 1].position = sf::Vector2f(m_chart_rect.left + (i + 1) * (m_chart_rect.width / (n_lines_x + 1)), m_chart_rect.top + m_chart_rect.height);
        m_grid[2 * i + 1].color    = color;
    }

    // horizontal lines
    for (int i = n_lines_x, j = 0; i < (n_lines_x + n_lines_y); ++i, ++j) {
        m_grid[2 * i].position     = sf::Vector2f(m_chart_rect.left, m_chart_rect.top + ((j + 1) * (m_chart_rect.height / (n_lines_y + 1))));
        m_grid[2 * i].color        = color;
        m_grid[2 * i + 1].position = sf::Vector2f(m_chart_rect.left + m_chart_rect.width, m_chart_rect.top + ((j + 1) * (m_chart_rect.height / (n_lines_y + 1))));
        m_grid[2 * i + 1].color    = color;
    }
}

void Chart::CreateAxisMarkers()
{
    CreateAxisX();
    CreateAxisY();
}

void Chart::CreateAxisY()
{
    sf::FloatRect rect = m_chart_rect;
    int           n    = m_num_grid_lines_y + 2; // + 2 is for 0 and max

    m_y_axis_markers.clear();
    m_y_axis_markers.reserve(n);
    for (int i = 0; i < n; ++i) {
        m_y_axis_markers.push_back(sf::Text());
        auto& marker = m_y_axis_markers[m_y_axis_markers.size() - 1];
        marker.setFont(m_font);
        marker.setFillColor(sf::Color::Black);
        marker.setCharacterSize(18);
        float             tmp = i * m_max_val / (n - 1);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << tmp;
        marker.setString(ss.str());
        marker.setOrigin(marker.getLocalBounds().left + marker.getLocalBounds().width / 2.f,
                         marker.getLocalBounds().top + marker.getLocalBounds().height / 2.f);
        marker.setPosition(rect.left - marker.getLocalBounds().width / 2 - 3, rect.top + rect.height - i * rect.height / (n - 1));
    }
}

void Chart::CreateAxisX()
{
    sf::FloatRect rect = m_chart_rect;
    int           n    = m_num_grid_lines_x + 2; // + 2 is for 0 and max

    m_x_axis_markers.clear();
    m_x_axis_markers.reserve(n);
    for (int i = 0; i < n; ++i) {
        m_x_axis_markers.push_back(sf::Text());
        auto& marker = m_x_axis_markers.back();
        marker.setFont(m_font);
        marker.setFillColor(sf::Color::Black);
        marker.setCharacterSize(18);
        // X markers will be in minutes
        float tmpf = i * ((static_cast<float>(m_sampling_period_ms) / (60 * 1000)) * m_chart_rect.width) / (n - 1);
        int   tmpi = std::round(tmpf);
        marker.setString(std::to_string(tmpi));
        marker.setOrigin(marker.getLocalBounds().left + marker.getLocalBounds().width / 2.f,
                         marker.getLocalBounds().top + marker.getLocalBounds().height / 2.f);
        marker.setPosition(rect.left + i * rect.width / (n - 1), rect.top + rect.height + marker.getLocalBounds().height);
    }
}

void Chart::SetAxisX(int startx)
{
    for (int i = 0; i < m_x_axis_markers.size(); ++i) {
        auto& marker = m_x_axis_markers[i];
        // X markers will be in minutes
        // If e.g. sampling period is 3.6s, then with graph region width = 1000, we have exactly 1 hour long graphing region.
        float tmpf = startx * (static_cast<float>(m_sampling_period_ms) / (60 * 1000)) +
                     i * ((static_cast<float>(m_sampling_period_ms) / (60 * 1000)) * m_chart_rect.width) / (m_x_axis_markers.size() - 1);
        int tmpi = std::round(tmpf);
        marker.setString(std::to_string(tmpi));
    }
}

const sf::FloatRect& Chart::GraphRegion()
{
    return m_chart_rect;
}

void Chart::OnKeyPress(const chart_callback_type& f)
{
    m_onKeyPress = f;
}

void Chart::ToggleDrawChartSignal(int idx)
{
    if (idx >= 0 && idx < m_chart_signals.size())
        m_chart_signals[idx]->enabled = !m_chart_signals[idx]->enabled;
}

void Chart::ToggleDrawAllChartSignals()
{
    m_draw_all_chart_signals = !m_draw_all_chart_signals;
    for (auto& sig : m_chart_signals)
        sig->enabled = m_draw_all_chart_signals;
}

void Chart::ClearChartSignals()
{
    for (auto& cs : m_chart_signals)
        cs->Clear();
}

void Chart::SetSamplingPeriod(uint32_t sampling_period_ms)
{
    m_sampling_period_ms = sampling_period_ms;
}