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

constexpr int default_number_of_clouds{3};
constexpr double default_size{0.8 * cube::cube_size_1d};
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
    hue_.add({0.0, color_transparent})
        .add({0.3, read_property(cloud_color, default_color)})
        .add({1.0, color_white});

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
        double const fade_scalar = cloud.in_fader->value() * cloud.out_fader->value();
        double const alpha_and_size_scalar = map(fade_scalar, 0.0, 1.0, 0.6, 1.0);

        p.set_color(hue_(fade_scalar).vec() * alpha_vec(alpha_and_size_scalar));
        p.scatter(cloud.voxel, cloud_size_ * alpha_and_size_scalar);
    }
}

void lightning::stop()
{
    scene_.stop();

    for (auto & cloud : clouds_) {
        cloud.in_fader.reset();
        cloud.out_fader.reset();
    }
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
    auto const fade_in_time = milliseconds(750 + std::rand() % 1250);
    auto const fade_in_resolution = static_cast<unsigned int>(fade_in_time / animation_scene_interval);

    c.voxel = random_voxel();
    c.in_fader = std::make_unique<ease_in_bounce>(context(), easing_config{{0.0, 1.0}, fade_in_resolution, fade_in_time},
        [this, &c]() { c.out_fader->start(); });
    c.out_fader = std::make_unique<ease_out_sine>(context(), easing_config{{1.0, 0.0}, fade_in_resolution / 8, fade_in_time / 8},
        [this, &c]() { spawn_cloud(c); });
    c.in_fader->start();
}

} // End of namespace
