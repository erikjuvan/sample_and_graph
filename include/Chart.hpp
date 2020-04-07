#pragma once

#include "Device.hpp"
#include "lsignal.hpp"
#include <memory>
#include <mygui/Object.hpp>

class Signal : public sf::Drawable
{
public:
    Signal(const sf::FloatRect& region) :
        m_graph_region(region)
    {
        m_curve.setPrimitiveType(sf::PrimitiveType::LineStrip);
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (enabled)
            target.draw(m_curve);
    }

    //void        SetColor(sf::Color const& col);
    const auto& Data() const { return m_data; }
    void        Append(std::vector<uint32_t> const& data)
    {
        const float y_zero = m_graph_region.top + m_graph_region.height;

        for (int i = m_curve.getVertexCount() < m_graph_region.width ? 0 : m_curve.getVertexCount() - m_graph_region.width; i < m_curve.getVertexCount(); ++i) {
            auto& vrtx = m_curve[i];
            if (vrtx.position.x > m_graph_region.left) {
                vrtx.position.x -= data.size();
                if (vrtx.position.x <= m_graph_region.left)
                    vrtx.color = sf::Color(0, 0, 0, 0); // hide vertex
            }
        }

        int xpos = m_graph_region.left + m_graph_region.width - 1 - data.size();
        for (auto d : data) {
            m_data.push_back(d);
            m_curve.append({sf::Vector2f(xpos++, y_zero - (d / 1300.f /*max_val*/) * m_graph_region.height + 1), sf::Color::Black});
        }
    }

    void Clear()
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

    void Update(std::vector<BaseDevice const*> const& devices);
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

    // Signals
    lsignal::signal<void(std::vector<std::shared_ptr<Signal>> const&)> signal_configured;
};
