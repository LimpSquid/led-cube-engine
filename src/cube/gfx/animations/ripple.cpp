#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;
using std::operator""s;

namespace
{

struct ripple :
    configurable_animation
{
    PROPERTY_ENUM
    (
        ripple_wave_time_ms,    // Time in milliseconds to complete one ripple period
        ripple_gradient,        // Ripple gradient
        ripple_length,          // Ripple length
        ripple_drift_factor_x,  // Drift factor in the X direction
        ripple_drift_factor_y,  // Drift factor in the Y direction
    )

    ripple(engine_context & context);

    void start() override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    json_or_error_t properties_to_json() const override;
    property_pairs_or_error_t properties_from_json(nlohmann::json const & json) const override;

    gradient gradient_;
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

void ripple::start()
{
    auto const wave_time = read_property(ripple_wave_time_ms, default_wave_time);

    gradient_ = read_property(ripple_gradient, default_gradient);
    length_ = read_property(ripple_length, default_length);
    omega_ = (2.0 * M_PI) / static_cast<double>(wave_time.count());
    time_ = rand(range{0, UINT16_MAX});
}

void ripple::scene_tick(milliseconds dt)
{
    time_ += static_cast<int>(dt.count());
}

void ripple::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();
    p.set_color(color_red);

    double const drift_factor_x = read_property(ripple_drift_factor_x, default_drift_factor);
    double const drift_factor_y = read_property(ripple_drift_factor_y, default_drift_factor);
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

            p.set_color(gradient_(map(z1, unit_circle_range, gradient_pos_range)));
            p.draw({x, y, z});
        }
    }
}

json_or_error_t ripple::properties_to_json() const
{
    return nlohmann::json {
        make_json(ripple_wave_time_ms, default_wave_time),
        make_json(ripple_length, default_length),
        make_json(ripple_gradient, default_gradient),
        make_json(ripple_drift_factor_x, default_drift_factor),
        make_json(ripple_drift_factor_y, default_drift_factor),
    };
}

property_pairs_or_error_t ripple::properties_from_json(nlohmann::json const & json) const
{
    auto const wave_time = parse_field(json, ripple_wave_time_ms, default_wave_time);
    if (wave_time == 0ms)
        return unexpected_error{"Field '"s + to_string(ripple_wave_time_ms) + "' cannot be 0ms"};

    return property_pairs_t {
        make_property(ripple_wave_time_ms, std::move(wave_time)),
        make_property(json, ripple_length, default_length),
        make_property(json, ripple_gradient, default_gradient),
        make_property(json, ripple_drift_factor_x, default_drift_factor),
        make_property(json, ripple_drift_factor_y, default_drift_factor),
    };
}

} // End of namespace
