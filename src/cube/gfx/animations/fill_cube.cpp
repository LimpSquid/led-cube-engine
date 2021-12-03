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
    config.time_step_interval = read_property<seconds>(property_cycle_interval_sec, 1s);
}

void fill_cube::time_step()
{
    update();
}

void fill_cube::paint(graphics_device & device)
{
    painter p(device);

    color_t r = static_cast<color_t>(rand() % color_max_value + 1);
    color_t g = static_cast<color_t>(rand() % color_max_value + 1);
    color_t b = static_cast<color_t>(rand() % color_max_value + 1);

    p.set_color({ r, g, b });
    p.fill_canvas();
}
