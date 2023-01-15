#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>
#include <cube/core/logging.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;
using std::operator""s;

namespace
{

struct ripple :
    configurable_animation
{
    ripple(engine_context & context);

    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    gradient gradient_;
    std::optional<ease_in_sine> fade_in_;
    std::optional<ease_out_sine> fade_out_;
    int time_;
    double omega_;
    double length_;
};

animation_publisher<ripple> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr milliseconds default_wave_time{1500ms};
constexpr double default_length{3.0};
constexpr double default_drift_factor{0.0};
gradient const default_gradient
{
    {0.00, color_cyan},
    {0.25, color_yellow},
    {0.75, color_red},
    {1.00, color_magenta},
};

ripple::ripple(engine_context & context) :
    configurable_animation(context)
{ }

void ripple::state_changed(animation_state state)
{
    switch (state) {
        case running: {
            auto wave_time = read_property<milliseconds>("ripple_wave_time_ms");
            if (wave_time <= 0ms) {
                LOG_WRN("Ignoring property 'ripple_wave_time_ms', must be > 0ms.");
                wave_time = default_wave_time;
            }

            gradient_ = read_property<gradient>("ripple_gradient");
            length_ = read_property<double>("ripple_length");
            omega_ = (2.0 * M_PI) / static_cast<double>(wave_time.count());
            time_ = rand(range{0, UINT16_MAX});

            fade_in_.emplace(context(), easing_config{{0.0, 1.0}, 20, get_transition_time()});
            fade_out_.emplace(context(), easing_config{{1.0, 0.0}, 20, get_transition_time()});

            fade_in_->start();
            break;
        }
        case stopping:
            fade_out_->start();
            break;
        case stopped:
            fade_in_->stop();
            fade_out_->stop();
            break;
    }
}

void ripple::scene_tick(milliseconds dt)
{
    time_ += static_cast<int>(dt.count());
}

void ripple::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    auto const drift_factor_x = read_property<double>("ripple_drift_factor_x");
    auto const drift_factor_y = read_property<double>("ripple_drift_factor_y");
    auto const compute_range = [&](double drift_factor) -> range<double> {
        if (equal(drift_factor, 0.0))
            return {-1.0, 1.0};

        double drift = std::sin(time_ * omega_ * drift_factor);
        return {
            map(drift, unit_circle_range, range{-1.0, -0.2}),
            map(drift, unit_circle_range, range{ 0.2,  1.0})
        };
    };

    auto drift_range_x = compute_range(drift_factor_x);
    auto drift_range_y = compute_range(drift_factor_y);

    for (int x = 0; x < cube::cube_size_1d; ++x) {
        double x1 = map(x, cube_axis_range, drift_range_x);
        for (int y = 0; y < cube::cube_size_1d; ++y) {
            double y1 = map(y, cube_axis_range, drift_range_y);
            double z1 = std::sin(time_ * omega_ + length_ * std::sqrt(x1 * x1 + y1 * y1));
            int z = map(z1, unit_circle_range, cube_axis_range);

            auto c = gradient_(map(z1, unit_circle_range, gradient_pos_range)).vec();
            c *= rgb_vec(fade_in_->value());
            c *= rgb_vec(fade_out_->value());

            p.set_color(c);
            p.draw({x, y, z});
        }
    }
}

std::unordered_map<std::string, property_value_t> ripple::extra_properties() const
{
    return {
        {"ripple_wave_time_ms", default_wave_time},
        {"ripple_length", default_length},
        {"ripple_gradient", default_gradient},
        {"ripple_drift_factor_x", default_drift_factor},
        {"ripple_drift_factor_y", default_drift_factor},
    };
}

} // End of namespace
