#include <cube/gfx/animations/helix.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

constexpr range cube_axis_range = {cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr color default_color = color_cyan;

} // End of namespace

namespace cube::gfx::animations
{

helix::helix(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this]() { step_++; }),
    fader_(context, {0.1, 1.0, 10, 1000ms})
{ }

void helix::start()
{
    int step_interval = read_property(helix_rotation_time_ms, 1500ms) / animation_scene_interval;

    hue_.add({0.0, read_property(color_gradient_start, default_color)});
    hue_.add({1.0, read_property(color_gradient_end, !default_color)});
    thickness_ = read_property(helix_thickness, 2.5);
    length_ =  2.0 * M_PI * read_property(helix_length, 0.875);
    omega_ = (2.0 * M_PI) / step_interval;
    step_ = std::rand() % UINT16_MAX;

    scene_.start();
    fader_.start();
}

void helix::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    double phase_shift_sin_factor = read_property(helix_phase_shift_sin_factor, 0.0);
    double phase_shift_cos_factor = read_property(helix_phase_shift_cos_factor, 0.0);

    for (int y = 0; y < cube_size_1d; y++) {
        double phase_shift = map(y, cube_axis_range, range(0.0, length_));
        double x1 = std::sin(step_ * omega_ + phase_shift * std::cos(step_ * omega_ * phase_shift_sin_factor));
        double z1 = std::cos(step_ * omega_ + phase_shift * std::cos(step_ * omega_ * phase_shift_cos_factor));
        int x = map(x1, unit_circle_range, cube_axis_range);
        int z = map(z1, unit_circle_range, cube_axis_range);

        p.set_color(hue_(abs_sin(step_ * omega_ + 0.5 * phase_shift)).vec() * rgb_vec(fader_.value()));
        p.scatter({x, y, z}, thickness_);
    }
}

void helix::stop()
{
    scene_.stop();
}

} // End of namespace
