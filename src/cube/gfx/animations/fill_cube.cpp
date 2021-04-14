#include <gfx/animations/fill_cube.h>
#include <core/painter.h>
#include <cstdlib>

using namespace cube::gfx::animations;
using namespace cube::core;
using namespace std::chrono;

void fill_cube::configure(animation_config &config)
{
    using namespace std::chrono_literals;

    config.time_step_ms = 100ms;
}

void fill_cube::time_step()
{
    using namespace std::chrono_literals;

    const animation_config &cfg = config();
    const seconds cycle_interval_sec = read_property<seconds>(property_cycle_interval_sec, 5s);

    elapsed_ms_ += cfg.time_step_ms;
    if(elapsed_ms_ >= cycle_interval_sec) {
        update();
        elapsed_ms_ = 0ms;
    }
}

void fill_cube::paint(graphics_device &device)
{
    painter p(device);

    using color_t = painter::color_type::value_type;
    color_t r = static_cast<color_t>(rand() % painter::color_type::max());
    color_t g = static_cast<color_t>(rand() % painter::color_type::max());
    color_t b = static_cast<color_t>(rand() % painter::color_type::max());

    p.fill_canvas({ r, g, b });
}