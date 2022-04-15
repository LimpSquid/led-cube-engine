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
    )

    ripple(engine_context & context);

    void start() override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    json_or_error_t properties_to_json() const override;
    property_pairs_or_error_t properties_from_json(nlohmann::json const & json) const override;

    gradient gradient_;
    int step_;
    double omega_;
    double length_;
};

animation_publisher<ripple> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr milliseconds default_wave_time{1500ms};
constexpr double default_length{3.0};
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
    int step_interval = static_cast<int>(read_property(ripple_wave_time_ms, default_wave_time) / cube::animation_scene_interval);

    gradient_ = read_property(ripple_gradient, default_gradient);
    length_ = read_property(ripple_length, default_length);
    omega_ = (2.0 * M_PI) / step_interval;
    step_ = rand(range{0, UINT16_MAX});
}

void ripple::scene_tick(milliseconds)
{
    step_++;
}

void ripple::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();
    p.set_color(color_red);

    for (int x = 0; x < cube::cube_size_1d; ++x) {
        double x1 = map(x, cube_axis_range, unit_circle_range);
        for (int y = 0; y < cube::cube_size_1d; ++y) {
            double y1 = map(y, cube_axis_range, unit_circle_range);
            double z1 = std::sin(step_ * omega_ + length_ * std::sqrt(x1 * x1 + y1 * y1));
            int z = map(z1, unit_circle_range, cube_axis_range);

            p.set_color(gradient_(map(z1, unit_circle_range, gradient_pos_range)));
            p.draw({x, y, z});
        }
    }
}

json_or_error_t ripple::properties_to_json() const
{
    return nlohmann::json {
        property_to_json(ripple_wave_time_ms, default_wave_time),
        property_to_json(ripple_length, default_length),
        property_to_json(ripple_gradient, default_gradient),
    };
}

property_pairs_or_error_t ripple::properties_from_json(nlohmann::json const & json) const
{
    auto wave_time = parse_field(json, ripple_wave_time_ms, default_wave_time);
    if (wave_time < cube::animation_scene_interval)
        return unexpected_error{"Field '"s + to_string(ripple_wave_time_ms) + "' must be atleast "
            + std::to_string(cube::animation_scene_interval.count()) + "ms"};

    return property_pairs_t {
        make_property(ripple_wave_time_ms, std::move(wave_time)),
        property_from_json(json, ripple_length, default_length),
        property_from_json(json, ripple_gradient, default_gradient),
    };
}

} // End of namespace
