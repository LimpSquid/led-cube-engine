#include <cube/gfx/animations/fill_cube.hpp>
#include <cube/core/painter.hpp>

using namespace cube::core;
using namespace std::chrono;

namespace cube::gfx::animations
{

fill_cube::fill_cube(engine_context & context) :
    configurable_animation(context),
    update_timer_(context, [this](auto, auto) { update(); })
{ }

void fill_cube::start()
{
    update_timer_.start(read_property(cycle_interval_sec, 5s));
}

void fill_cube::paint(graphics_device & device)
{
    painter p(device);

    color_t r = read_property(disable_red, false) ? 0 : static_cast<color_t>(rand() % color_max_value + 1);
    color_t g = read_property(disable_green, false) ? 0 : static_cast<color_t>(rand() % color_max_value + 1);
    color_t b = read_property(disable_blue, false) ? 0 : static_cast<color_t>(rand() % color_max_value + 1);

    p.set_color({r, g, b});
    p.fill_canvas();
}

void fill_cube::stop()
{
    update_timer_.stop();
}

} // End of namespace
