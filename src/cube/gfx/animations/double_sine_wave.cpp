#include <cube/gfx/animations/double_sine_wave.hpp>
#include <cube/core/painter.hpp>
#include <cube/specs.hpp>
#include <cmath>

#include <cube/core/gradient.hpp>

using namespace cube::core;
using namespace std::chrono;

namespace
{

constexpr double sine_offset = (cube::cube_size_1d - 1) / 2.0;
constexpr double hue_step = 1.0 / cube::cube_size_1d;

} // end of namespace

namespace cube::gfx::animations
{

void double_sine_wave::configure(animation_config & config)
{
    config.time_step_interval = read_property(frame_rate_ms, 50ms);

    init_waves();
}

void double_sine_wave::time_step()
{
    for (wave & w : waves)
        w.time_count = (w.time_count + 1) % w.period;
    update();
}

void double_sine_wave::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (wave const & w : waves) {
        gradient g;
        g.add({0.0, !w.color});
        g.add({1.0, w.color});

        for (int i = w.time_count; i < (w.time_count + cube_size_1d); ++i) {
            p.set_color(g(cos(i * w.omega)));

            int z = round(sine_offset * sin(i * w.omega) + sine_offset);
            int x = i - w.time_count;

            for(int y = 0; y < cube_size_1d; ++y)
                p.draw({x, y, z});
        }
    }
}

void double_sine_wave::init_waves()
{
    waves[0].time_count = 0;
    waves[0].period = read_property(period_wave_1, int(2 * cube_size_1d));
    waves[0].color = read_property(color_wave_1, color(255, 0, 0));
    waves[0].omega = (2.0 * M_PI) / waves[0].period;

    waves[1].time_count = 0;
    waves[1].period = read_property(period_wave_2, int(1.75 * cube_size_1d));
    waves[1].color = read_property(color_wave_2, color(0, 255, 0));
    waves[1].omega = (2.0 * M_PI) / waves[1].period;
}

} // end of namespace
