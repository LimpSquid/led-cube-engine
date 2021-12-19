#include <cube/gfx/animations/helix.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{


} // End of namespace

namespace cube::gfx::animations
{

helix::helix(engine_context & context) :
    configurable_animation(context)
{ }

void helix::configure()
{
    tick_sub_ = tick_subscription::create(
        context(),
        100ms,
        [this](auto, auto) {
            update();
        }
    );
}

void helix::paint(graphics_device & device)
{
    constexpr int half_cube_size = (cube_size_1d / 2);

    painter p(device);
    p.wipe_canvas();

    p.set_color(color_red);
}

void helix::stop()
{
    tick_sub_.reset();
}

} // End of namespace
