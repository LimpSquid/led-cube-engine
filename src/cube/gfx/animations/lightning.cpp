#include <cube/gfx/animations/lightning.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>
#include <cube/core/json_util.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::lightning> const publisher{"lightning"};

constexpr int default_number_of_clouds{5};
constexpr double default_size{0.625 * cube::cube_size_1d};
constexpr color default_color{color_blue};

} // End of namespace

namespace cube::gfx::animations
{

lightning::lightning(engine_context & context) :
    configurable_animation(context),
    scene_(*this)
{ }

void lightning::start()
{
    cloud_size_ = read_property(cloud_size, default_size);
    hue_.add({0.0, read_property(cloud_color, default_color)});
    hue_.add({0.75, read_property(cloud_color, default_color)});
    hue_.add({1.0, color_white});

    int num_clouds = read_property(number_of_clouds, default_number_of_clouds);
    clouds_.resize(num_clouds);
    for (auto & cloud : clouds_)
        spawn_cloud(cloud);
    scene_.start();
}

void lightning::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (auto const & cloud : clouds_) {
        p.set_color(hue_(cloud.fader->value()).vec() * cloud.fader->value());
        p.scatter(cloud.voxel, cloud_size_);
    }

    // p.set_color(color_blue);
    // p.scatter({cube_size_1d/2,cube_size_1d/2,cube_size_1d/2}, cloud_size_);
}

void lightning::stop()
{
    scene_.stop();

    for (auto & cloud : clouds_)
        cloud.fader.reset();
}

nlohmann::json lightning::properties_to_json() const
{
    return {
        make_field(number_of_clouds, read_property(number_of_clouds, default_number_of_clouds)),
        make_field(cloud_size, read_property(cloud_size, default_size)),
        make_field(cloud_color, read_property(cloud_color, default_color)),
    };
}

std::vector<lightning::property_pair_t> lightning::properties_from_json(nlohmann::json const & json) const
{
    return {
        {number_of_clouds, parse_field(json, number_of_clouds, default_number_of_clouds)},
        {cloud_size, parse_field(json, cloud_size, default_size)},
        {cloud_color, parse_field(json, cloud_color, default_color)},
    };
}

void lightning::spawn_cloud(cloud & c)
{
    easing_config config =
    {
        0.0,
        1.0,
        35,
        milliseconds(500 + std::rand() % 1500)
    };

    c.voxel = random_voxel();
    c.fader = std::make_unique<ease_in_bounce>(context(), std::move(config),
        [this, &c]() { spawn_cloud(c); });
    c.fader->start();
}

} // End of namespace
