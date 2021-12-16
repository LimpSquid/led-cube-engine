#include <cube/gfx/animations/double_sine_wave.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/gradient.hpp>
#include <cube/specs.hpp>

using namespace cube::core;
using namespace std::chrono;

namespace
{

constexpr double sine_offset = (cube::cube_size_1d - 1) / 2.0;
constexpr color default_color = color_magenta;

} // End of namespace

namespace cube::gfx::animations
{

void double_sine_wave::configure(animation_config & config)
{
    auto const gradient_start = read_property(color_gradient_start, default_color);
    auto const gradient_end = read_property(color_gradient_end, !default_color);
    period_ = std::max(1, read_property(wave_period, int(2 * cube_size_1d)));
    omega_ = (2.0 * M_PI) / period_;

    waves_[0].time_count = 0;
    waves_[0].gradient_start = gradient_start;
    waves_[0].gradient_end = gradient_end;

    waves_[1].time_count = period_ / 3; // Shift 120 degrees
    waves_[1].gradient_start = gradient_end; // Invert color of 2nd wave
    waves_[1].gradient_end = gradient_start;

    config.time_step_interval = read_property(wave_period_time_ms, 1500ms) / period_;
}

void double_sine_wave::time_step()
{
    for (wave & w : waves_)
        w.time_count = (w.time_count + 1) % period_;
    update();
}

void double_sine_wave::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (wave const & w : waves_) {
        gradient g({
            {0.0, w.gradient_start},
            {1.0, w.gradient_end},
        });

        for (int i = w.time_count; i < (w.time_count + cube_size_1d); ++i) {
            p.set_color(g(std::fabs(std::cos(i * omega_))));

            int z = std::round(sine_offset * std::sin(i * omega_) + sine_offset);
            int x = i - w.time_count;

            for (int y = 0; y < cube_size_1d; ++y)
                p.draw({x, y, z});
        }
    }
}

} // End of namespace
