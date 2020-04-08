#pragma once

#include "Device.hpp"
#include "lsignal.hpp"
#include <memory>
#include <mygui/Object.hpp>
#include <mygui/ResourceManager.hpp>

class ChartSignal : public sf::Drawable
{
public:
    ChartSignal(const sf::FloatRect& region) :
        m_graph_region(region)
    {
        m_font.loadFromFile(mygui::ResourceManager::GetSystemFontName());
        m_text.setFont(m_font);
        m_text.setCharacterSize(12);
        m_text.setColor(sf::Color::Black);
        m_curve.resize(static_cast<int>(region.width) - 1);
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (enabled) {
            target.draw(&m_curve[0], m_curve.size(), sf::PrimitiveType::LineStrip, states);
            if (m_curve.back().position.y > 0)
                target.draw(m_text);
        }
    }

    const auto& Data() const { return m_data; }
    void        Append(std::vector<float> const& data)
    {
        const float y_zero = m_graph_region.top + m_graph_region.height;

        m_curve.erase(m_curve.begin(), m_curve.begin() + data.size());
        for (auto d : data) {
            m_data.push_back(d);
            m_curve.push_back({sf::Vector2f(0, y_zero - (d / 100.f /*max_val*/) * m_graph_region.height + 1), sf::Color::Black});
        }

        int startx = m_graph_region.left + 1;
        for (int i = 0; i < m_curve.size(); ++i)
            m_curve[i].position.x = startx + i;

        auto pos = m_curve.back().position;
        m_text.setPosition({pos.x + 5, pos.y - m_text_center_pos});
    }

    void Clear()
    {
        m_data.clear();
        m_name.clear();
        m_text.setString("");
    }

    void Name(std::string name)
    {
        m_name = name;
        m_text.setString(name);
        m_text_center_pos = m_text.getGlobalBounds().height / 2 + 2;
    }
    std::string Name() { return m_name; }

    bool enabled{true};

private:
    int m_sample_period_ms{0};

    std::vector<float>      m_data;
    std::vector<sf::Vertex> m_curve;
    sf::FloatRect           m_graph_region;

    std::string m_name;
    int         m_text_center_pos;
    sf::Font    m_font;
    sf::Text    m_text;
};

class Chart : public mygui::Object
{
private:
    using chart_callback_type = std::function<void(const sf::Event&)>;

    const int m_margin{20};

    void ConvertData(std::vector<float>& data);

    sf::RectangleShape m_background;
    sf::RectangleShape m_chart_region;
    sf::FloatRect      m_chart_rect;
    sf::VertexArray    m_outline;
    sf::VertexArray    m_axes;
    sf::VertexArray    m_grid;
    int                m_num_grid_lines{0};
    sf::Text           m_x_axis;
    sf::Text           m_y_axis;
    sf::Text           m_title;

    sf::Font m_font;

    std::vector<sf::Text> m_x_axis_markers;
    std::vector<sf::Text> m_y_axis_markers;

    std::vector<std::shared_ptr<ChartSignal>> m_chart_signals;
    bool                                      m_draw_all_chart_signals = true;

    std::shared_ptr<float> m_max_val;

    int m_num_of_points;

    bool m_mouseover;

    chart_callback_type m_onKeyPress{nullptr};

public:
    Chart(int x, int y, int w, int h, int num_of_points, float max_val);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    virtual void Handle(const sf::Event& event) override;

    virtual void Enabled(bool enabled) override;
    virtual bool Enabled() const override;

    void AddChartSignal(std::shared_ptr<ChartSignal> const& csignal);
    void ChangeChartSignal(int idx, std::shared_ptr<ChartSignal> const& csignal);

    void Update(std::vector<BaseDevice const*> const& devices);
    void LoadDevices(std::vector<BaseDevice const*> const& devices);

    // n_lines - number of one type of lines (vertical or horizontal), there are same number of other lines
    void                         CreateGrid(int n_lines);
    void                         CreateAxisMarkers();
    const sf::FloatRect&         GraphRegion();
    std::shared_ptr<float const> MaxVal();
    void                         ToggleDrawChartSignal(int idx);
    void                         ToggleDrawAllChartSignals();

    // Actions
    void OnKeyPress(const chart_callback_type& f);

    // Signals
    lsignal::signal<void(std::vector<std::shared_ptr<ChartSignal>> const&)> signal_chart_signals_configured;
};
