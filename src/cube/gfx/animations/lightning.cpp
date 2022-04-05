#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct cloud
{
    voxel_t voxel;
    std::unique_ptr<ease_in_bounce> in_fader;
    std::unique_ptr<ease_out_sine> out_fader;
};

struct lightning :
    configurable_animation
{
    PROPERTY_ENUM
    (
        number_of_clouds,       // Number of clouds in the cube
        cloud_radius,           // Radius of the cloud
        cloud_gradient,         // Gradient of the clouds
    )

    lightning(engine_context & context);

    void start() override;
    void paint(graphics_device & device) override;
    void stop() override;
    nlohmann::json properties_to_json() const override;
    std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    void spawn_cloud(cloud & c);

    std::vector<cloud> clouds_;
    gradient cloud_gradient_;
    int cloud_radius_;
};

animation_publisher<lightning> const publisher;

constexpr unsigned int default_number_of_clouds{3};
constexpr int default_radius{4 * cube::cube_size_1d / 5};
gradient const default_gradient
{
    {0.00, color_transparent},
    {0.50, color_blue},
    {1.00, color_white},
};

lightning::lightning(engine_context & context) :
    configurable_animation(context)
{ }

void lightning::start()
{
    cloud_gradient_ = read_property(cloud_gradient, default_gradient);
    cloud_radius_ = read_property(cloud_radius, default_radius);

    unsigned int num_clouds = read_property(number_of_clouds, default_number_of_clouds);
    clouds_.resize(num_clouds);
    for (auto & cloud : clouds_)
        spawn_cloud(cloud);
}

void lightning::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (auto const & cloud : clouds_) {
        double const fade_scalar = cloud.in_fader->value() * cloud.out_fader->value();
        int const radius = static_cast<int>(std::round(cloud_radius_ * map(fade_scalar, 0.0, 1.0, 0.5, 1.0)));

        p.set_color(cloud_gradient_(fade_scalar).vec() * alpha_vec(map(fade_scalar, 0.0, 1.0, 0.6, 1.0)));
        p.sphere(cloud.voxel, radius);
    }
}

void lightning::stop()
{
    for (auto & cloud : clouds_) {
        cloud.in_fader.reset();
        cloud.out_fader.reset();
    }
}

nlohmann::json lightning::properties_to_json() const
{
    return {
        to_json(number_of_clouds, default_number_of_clouds),
        to_json(cloud_radius, default_radius),
        to_json(cloud_gradient, default_gradient),
    };
}

std::vector<lightning::property_pair_t> lightning::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, number_of_clouds, default_number_of_clouds),
        from_json(json, cloud_radius, default_radius),
        from_json(json, cloud_gradient, default_gradient),
    };
}

void lightning::spawn_cloud(cloud & c)
{
    auto const fade_in_time = milliseconds(rand(range{750, 2000}));
    auto const fade_in_resolution = static_cast<unsigned int>(fade_in_time / cube::animation_scene_interval);

    c.voxel = random_voxel();
    c.in_fader = std::make_unique<ease_in_bounce>(context(), easing_config{{0.0, 1.0}, fade_in_resolution, fade_in_time},
        [&c]() { c.out_fader->start(); });
    c.out_fader = std::make_unique<ease_out_sine>(context(), easing_config{{1.0, 0.0}, fade_in_resolution / 8, fade_in_time / 8},
        [this, &c]() { spawn_cloud(c); });
    c.in_fader->start();
}

} // End of namespace
