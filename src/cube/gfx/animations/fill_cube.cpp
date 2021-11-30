#include <cube/gfx/animations/fill_cube.h>
#include <cube/core/painter.h>
#include <cube/core/color.h>
#include <cstdlib>

using namespace cube::gfx::animations;
using namespace cube::core;
using namespace std::chrono;
using namespace std::chrono_literals;

void fill_cube::configure(animation_config & config)
{
    config.time_step_interval = 100ms;
}

void fill_cube::time_step()
{
    animation_config const & cfg = config();
    seconds const cycle_interval_sec = read_property<seconds>(property_cycle_interval_sec, 5s);

    elapsed_ms_ += cfg.time_step_interval;
    if(elapsed_ms_ >= cycle_interval_sec) {
        update();
        elapsed_ms_ = 0ms;
    }
}

void fill_cube::paint(graphics_device & device)
{
    painter p(device);

    color_t r = static_cast<color_t>(rand() % color_max_value + 1);
    color_t g = static_cast<color_t>(rand() % color_max_value + 1);
    color_t b = static_cast<color_t>(rand() % color_max_value + 1);

    p.fill_canvas({ r, g, b });
}
