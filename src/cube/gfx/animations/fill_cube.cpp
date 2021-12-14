#include <cube/gfx/animations/fill_cube.hpp>
#include <cube/core/painter.hpp>
#include <cstdlib>

using namespace cube::core;
using namespace std::chrono;

namespace name
{

} // namespace name


namespace cube::gfx::animations
{

void fill_cube::configure(animation_config & config)
{
    config.time_step_interval = read_property(cycle_interval_sec, 5s);
}

void fill_cube::time_step()
{
    update();
}

void fill_cube::paint(graphics_device & device)
{
    painter p(device);

    color_t r = read_property(disable_red, false) ? 0 : static_cast<color_t>(rand() % color_max_value + 1);
    color_t g = read_property(disable_green, false) ? 0 : static_cast<color_t>(rand() % color_max_value + 1);
    color_t b = read_property(disable_blue, false) ? 0 : static_cast<color_t>(rand() % color_max_value + 1);

    p.set_color({ r, g, b });
    p.fill_canvas();
}

} // end of namespace
