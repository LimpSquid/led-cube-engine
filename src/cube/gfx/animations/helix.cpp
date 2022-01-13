#include <cube/gfx/animations/helix.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::helix> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr color default_color{color_cyan};
constexpr milliseconds default_rotation_time{1500ms};
constexpr int default_thickness{3};
constexpr double default_length{0.875};
constexpr double default_phase_shift_factor{0.0};

} // End of namespace

namespace cube::gfx::animations
{

helix::helix(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this](auto) { step_++; }),
    fader_(context, {{0.1, 1.0}, 10, 1000ms})
{ }

void helix::start()
{
    int step_interval = static_cast<int>(read_property(helix_rotation_time_ms, default_rotation_time) / animation_scene_interval);

    hue_.add({0.0, read_property(color_gradient_start, default_color)})
        .add({1.0, read_property(color_gradient_end, !default_color)});
    thickness_ = read_property(helix_thickness, default_thickness);
    length_ =  2.0 * M_PI * read_property(helix_length, default_length);
    omega_ = (2.0 * M_PI) / step_interval;
    step_ = std::rand() % UINT16_MAX;

    scene_.start();
    fader_.start();
}

void helix::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    double phase_shift_sin_factor = read_property(helix_phase_shift_sin_factor, default_phase_shift_factor);
    double phase_shift_cos_factor = read_property(helix_phase_shift_cos_factor, default_phase_shift_factor);

    for (int y = 0; y < cube_size_1d; y++) {
        double phase_shift = map(y, cube_axis_range, range(0.0, length_));
        double x1 = std::sin(step_ * omega_ + phase_shift * std::cos(step_ * omega_ * phase_shift_sin_factor));
        double z1 = std::cos(step_ * omega_ + phase_shift * std::cos(step_ * omega_ * phase_shift_cos_factor));
        int x = map(x1, unit_circle_range, cube_axis_range);
        int z = map(z1, unit_circle_range, cube_axis_range);

        p.set_color(hue_(abs_sin(step_ * omega_ + 0.5 * phase_shift)).vec() * rgb_vec(fader_.value()));
        p.sphere({x, y, z}, thickness_);
    }
}

void helix::stop()
{
    scene_.stop();
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
        to_json(color_gradient_start, default_color),
        to_json(color_gradient_end, default_color),
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
        from_json(json, color_gradient_start, default_color),
        from_json(json, color_gradient_end, default_color),
    };
}

} // End of namespace
