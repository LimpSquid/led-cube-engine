#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct helix :
    configurable_animation
{
    PROPERTY_ENUM
    (
        helix_rotation_time_ms,         // Time in milliseconds to complete one helix rotation
        helix_phase_shift_cos_factor,   // Just play with it and you'll see :-)
        helix_phase_shift_sin_factor,   // Same...
        helix_thickness,                // Cross-section radius of the helix
        helix_length,                   // Length of the helix
        helix_gradient,                 // Helix gradient
    )

    helix(engine_context & context);

    void start() override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    void stop() override;
    nlohmann::json properties_to_json() const override;
    std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    gradient gradient_;
    ease_in_sine fader_;
    int step_;
    int thickness_;
    double omega_;
    double length_;
};

animation_publisher<helix> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr milliseconds default_rotation_time{1500ms};
constexpr int default_thickness{3};
constexpr double default_length{0.875};
constexpr double default_phase_shift_factor{0.0};
gradient const default_gradient
{
    {0.00, color_cyan},
    {0.50, color_yellow},
    {1.00, color_magenta},
};

helix::helix(engine_context & context) :
    configurable_animation(context),
    fader_(context, {{0.1, 1.0}, 10, 1000ms})
{ }

void helix::start()
{
    int step_interval = static_cast<int>(read_property(helix_rotation_time_ms, default_rotation_time) / cube::animation_scene_interval);

    gradient_ = read_property(helix_gradient, default_gradient);
    thickness_ = read_property(helix_thickness, default_thickness);
    length_ =  2.0 * M_PI * read_property(helix_length, default_length);
    omega_ = (2.0 * M_PI) / step_interval;
    step_ = rand(range{0, UINT16_MAX});

    fader_.start();
}

void helix::scene_tick(milliseconds)
{
    step_++;
}

void helix::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    double phase_shift_sin_factor = read_property(helix_phase_shift_sin_factor, default_phase_shift_factor);
    double phase_shift_cos_factor = read_property(helix_phase_shift_cos_factor, default_phase_shift_factor);

    for (int y = 0; y < cube::cube_size_1d; y++) {
        double phase_shift = map(y, cube_axis_range, range(0.0, length_));
        double x1 = std::sin(step_ * omega_ + phase_shift * std::cos(step_ * omega_ * phase_shift_sin_factor));
        double z1 = std::cos(step_ * omega_ + phase_shift * std::cos(step_ * omega_ * phase_shift_cos_factor));
        int x = map(x1, unit_circle_range, cube_axis_range);
        int z = map(z1, unit_circle_range, cube_axis_range);

        p.set_color(gradient_(abs_sin(step_ * omega_ + 0.5 * phase_shift)).vec() * rgb_vec(fader_.value()));
        p.sphere({x, y, z}, thickness_);
    }
}

void helix::stop()
{
    fader_.stop();
}

nlohmann::json helix::properties_to_json() const
{
    return {
        to_json(helix_rotation_time_ms, default_rotation_time),
        to_json(helix_phase_shift_cos_factor, default_phase_shift_factor),
        to_json(helix_phase_shift_sin_factor, default_phase_shift_factor),
        to_json(helix_thickness, default_thickness),
        to_json(helix_length, default_length),
        to_json(helix_gradient, default_gradient),
    };
}

std::vector<helix::property_pair_t> helix::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, helix_rotation_time_ms, default_rotation_time),
        from_json(json, helix_phase_shift_cos_factor, default_phase_shift_factor),
        from_json(json, helix_phase_shift_sin_factor, default_phase_shift_factor),
        from_json(json, helix_thickness, default_thickness),
        from_json(json, helix_length, default_length),
        from_json(json, helix_gradient, default_gradient),
    };
}

} // End of namespace
