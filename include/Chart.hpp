#pragma once

#include <memory>
#include <mygui/Object.hpp>

class Signal : public sf::Drawable
{
public:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (m_draw)
            target.draw(m_curve);
    }

    void EnableDraw() { m_draw = true; }
    void DisableDraw() { m_draw = false; }
    //void        SetColor(sf::Color const& col);
    const auto& GetRXData() const { return m_rx_data; }
    void        ClearRXData() { m_rx_data.clear(); }

private:
    int m_sample_period_ms{0};

    std::vector<uint32_t> m_rx_data;
    sf::VertexArray       m_curve;
    sf::FloatRect         m_graph_region;

    bool m_draw{true};
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
    std::vector<bool>                    m_draw_signal;
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
