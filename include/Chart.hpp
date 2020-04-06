#pragma once

#include "Device.hpp"
#include <memory>
#include <mygui/Object.hpp>

class Signal : public sf::Drawable
{
public:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (enabled)
            target.draw(m_curve);
    }

    //void        SetColor(sf::Color const& col);
    const auto& Data() const { return m_data; }
    void        Append(std::vector<uint32_t> const& data) { m_data.insert(m_data.end(), data.begin(), data.end()); }
    void        Clear()
    {
        m_data.clear();
        name.clear();
    }

    std::string name;
    bool        enabled{true};

private:
    int m_sample_period_ms{0};

    std::vector<uint32_t> m_data;
    sf::VertexArray       m_curve;
    sf::FloatRect         m_graph_region;
};

class Chart : public mygui::Object
{
private:
    using chart_callback_type = std::function<void(const sf::Event&)>;

    const int m_margin{20};

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

    std::vector<std::shared_ptr<Signal>> m_signals;
    bool                                 m_draw_all_signals = true;

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

    void AddSignal(std::shared_ptr<Signal> const& signal);
    void ChangeSignal(int idx, std::shared_ptr<Signal> const& signal);

    void Update();
    void LoadDevices(std::vector<BaseDevice const*> const& devices);

    // n_lines - number of one type of lines (vertical or horizontal), there are same number of other lines
    void                         CreateGrid(int n_lines);
    void                         CreateAxisMarkers();
    const sf::FloatRect&         GraphRegion();
    std::shared_ptr<float const> MaxVal();
    void                         ToggleDrawSignal(int idx);
    void                         ToggleDrawAllSignals();

    // Actions
    void OnKeyPress(const chart_callback_type& f);
};
