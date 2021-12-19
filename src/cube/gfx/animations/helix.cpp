#include <cube/gfx/animations/helix.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

constexpr Range helix_phase_shift_range = {0.0, 1.75 * M_PI}; // Show 7/8th of a helix
constexpr Range cube_axis_range = {cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr Range unit_circle_range = {-1.0, 1.0};
constexpr color default_color = color_cyan;

} // End of namespace

namespace cube::gfx::animations
{

helix::helix(engine_context & context) :
    configurable_animation(context),
    fader_(context, {0.1, 1.0, 10, 1000ms})
{ }

void helix::configure()
{
    hue_.add({0.0, read_property(color_gradient_start, default_color)});
    hue_.add({1.0, read_property(color_gradient_end, !default_color)});
    resolution_ = std::max(1, read_property(helix_resolution, 50));
    omega_ = (2.0 * M_PI) / resolution_;
    step_ = 0;

    tick_sub_ = tick_subscription::create(
        context(),
        read_property(helix_rotation_time_ms, 1500ms) / resolution_,
        [this](auto, auto) {
            step_++;
            update();
        }
    );

    fader_.start();
}

void helix::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    double phase_shift_sin_factor = read_property(helix_phase_shift_sin_factor, 0.0);
    double phase_shift_cos_factor = read_property(helix_phase_shift_cos_factor, 0.0);

    for (int y = 0; y < cube_size_1d; y++) {
        double phase_shift = map(y, cube_axis_range, helix_phase_shift_range);
        double x1 = std::sin(step_ * omega_ + phase_shift * std::cos(step_ * omega_ * phase_shift_sin_factor));
        double z1 = std::cos(step_ * omega_ + phase_shift * std::cos(step_ * omega_ * phase_shift_cos_factor));
        int x = map(x1, unit_circle_range, cube_axis_range);
        int z = map(z1, unit_circle_range, cube_axis_range);

        p.set_color(hue_(std::sin(std::abs(step_ * omega_ + phase_shift))).vec() * fader_.value());
        p.sphere({x, y, z}, 1.5);
    }
}

void helix::stop()
{
    tick_sub_.reset();
}

} // End of namespace
