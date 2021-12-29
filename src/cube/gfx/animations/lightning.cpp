#include <cube/gfx/animations/lightning.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>
#include <cube/core/json_util.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::lightning> const publisher = {"lightning"};

gradient const hue =
{
    {0.00, color_blue},
    {0.75, color_blue},
    {1.00, color_white},
};

} // End of namespace

namespace cube::gfx::animations
{

lightning::lightning(engine_context & context) :
    configurable_animation(context),
    scene_(*this)
{ }

void lightning::start()
{
    clouds_.resize(5);
    for (auto & cloud : clouds_)
        spawn_cloud(cloud);
    scene_.start();
}

void lightning::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (auto const & cloud : clouds_) {
        p.set_color(hue(cloud.fader->value()).vec() * cloud.fader->value());
        p.scatter(cloud.voxel, 10);
    }
}

void lightning::stop()
{
    scene_.stop();

    for (auto & cloud : clouds_)
        cloud.fader.reset();
}

nlohmann::json lightning::properties_to_json() const
{
    return { };
}

std::vector<lightning::property_pair> lightning::properties_from_json(nlohmann::json const & json) const
{
    return { };
}

void lightning::spawn_cloud(cloud & c)
{
    c.voxel = random_voxel();
    c.fader = std::make_unique<ease_in_bounce>(context(), easing_config{0.0, 1.0, 35, milliseconds(500 + std::rand() % 1500)},
        [this, &c]() { spawn_cloud(c); });
    c.fader->start();
}

} // End of namespace
