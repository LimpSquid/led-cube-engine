#include <cube/gfx/animations/fill_cube.hpp>
#include <cube/core/painter.hpp>

using namespace cube::core;
using namespace std::chrono;

namespace cube::gfx::animations
{

fill_cube::fill_cube(core::engine_context & context) :
    configurable_animation(context)
{ }

void fill_cube::configure()
{
    tick_sub_ = tick_subscription::create(
        context(),
        read_property(cycle_interval_sec, 5s),
        [this](auto, auto) { update(); }
    );
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
    tick_sub_.reset();
}

} // End of namespace
