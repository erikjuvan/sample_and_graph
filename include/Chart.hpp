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
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (enabled && m_curve.size() > 0) {
            target.draw(&m_curve[0], m_curve.size(), sf::PrimitiveType::LineStrip, states);
            if (m_curve.back().position.y > 0)
                target.draw(m_text);
        }
    }

    const auto& Data() const { return m_data; }

    void Append(std::vector<float> const& data)
    {
        for (auto d : data) {
            m_data.push_back(d);
        }

        if (!m_draw_index_overwrite && m_draw_index <= (m_data.size() - m_graph_region.width)) {
            int idx = m_data.size() - m_graph_region.width - 1;
            if (idx < 0)
                idx = 0;
            m_draw_index = idx;
        }

        UpdataCurve();
    }

    void Clear()
    {
        m_data.clear();
        auto size = m_curve.size();
        m_curve.clear();
        m_curve.resize(size);
        m_draw_index = {0};
    }

    void Name(std::string name)
    {
        m_name = name;
        m_text.setString(name);
        m_text_center_pos = m_text.getGlobalBounds().height / 2 + 2;
    }
    std::string Name() { return m_name; }

    bool enabled{true};

    void ChangeDrawIndex(int draw_index_delta)
    {
        m_draw_index += draw_index_delta;
        if (m_draw_index < 0)
            m_draw_index = 0;
        if (m_draw_index > m_data.size())
            m_draw_index = m_data.size() - 1;

        if (m_data.size() - m_draw_index < m_graph_region.width)
            m_draw_index_overwrite = false;
        else
            m_draw_index_overwrite = true;

        UpdataCurve();
    }

    int GetDrawIndex() const
    {
        return m_draw_index;
    }

private:
    void UpdataCurve()
    {
        if (m_data.size() <= 0)
            return;

        const float y_zero = m_graph_region.top + m_graph_region.height;
        int         startx = m_graph_region.left;

        m_curve.clear();
        for (auto it = m_data.begin() + m_draw_index; it != m_data.end() && m_curve.size() <= m_graph_region.width; ++it) {
            m_curve.push_back({sf::Vector2f(startx++, y_zero - (*it / 100.f /*max_val*/) * m_graph_region.height), sf::Color::Black});
        }

        // Update text positions
        if (m_curve.size() > 0) {
            auto pos = m_curve.back().position;
            m_text.setPosition({pos.x + 5, pos.y - m_text_center_pos});
        }
    }

private:
    int m_sampling_period_ms{0};

    std::vector<float>      m_data;
    int                     m_draw_index{0};
    bool                    m_draw_index_overwrite{false};
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
    int                m_num_grid_lines_x{0}, m_num_grid_lines_y{0};
    sf::Text           m_x_axis;
    sf::Text           m_y_axis;
    sf::Text           m_title;

    sf::Font m_font;

    std::vector<sf::Text> m_x_axis_markers;
    std::vector<sf::Text> m_y_axis_markers;

    std::vector<std::shared_ptr<ChartSignal>> m_chart_signals;
    bool                                      m_draw_all_chart_signals = true;

    float m_max_val;

    int m_num_of_points;

    int m_sampling_period_ms{0};

    bool m_mouseover;

    // Sliding mouse action
    int  m_mouse_drag_start_pos_x;
    bool m_holding_left_mouse_button{false};

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
    void                 CreateGrid(int n_lines_x, int n_lines_y);
    void                 CreateAxisMarkers();
    void                 CreateAxisX();
    void                 CreateAxisY();
    void                 SetAxisX(int startx);
    const sf::FloatRect& GraphRegion();
    void                 SetDrawChartSignal(int idx, bool on);
    bool                 ToggleDrawChartSignal(int idx);
    bool                 ToggleDrawAllChartSignals();

    void SetSamplingPeriod(uint32_t sampling_period_ms);

    void ClearChartSignals();

    // Actions
    void OnKeyPress(const chart_callback_type& f);

    // Signals
    lsignal::signal<void(std::vector<std::shared_ptr<ChartSignal>> const&)> signal_chart_signals_configured;
};
