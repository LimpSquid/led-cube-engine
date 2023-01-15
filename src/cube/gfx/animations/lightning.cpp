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
    lightning(engine_context & context);

    void state_changed(animation_state state) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

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

void lightning::state_changed(animation_state state)
{
    switch (state) {
        case running: {
            cloud_gradient_ = read_property<gradient>("cloud_gradient");
            cloud_radius_ = read_property<int>("cloud_radius");

            auto const num_clouds = read_property<unsigned int>("number_of_clouds");
            clouds_.resize(num_clouds);
            for (auto & cloud : clouds_)
                spawn_cloud(cloud);
            break;
        }
        case stopped:
            for (auto & cloud : clouds_) {
                cloud.in_fader.reset();
                cloud.out_fader.reset();
            }
            break;
        default:;
    }
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

std::unordered_map<std::string, property_value_t> lightning::extra_properties() const
{
    return {
        { "number_of_clouds", default_number_of_clouds },
        { "cloud_radius", default_radius },
        { "cloud_gradient", default_gradient },
    };
}

void lightning::spawn_cloud(cloud & c)
{
    constexpr int fade_out_factor = 8;
    constexpr int fade_time_min = std::max(750ms, fade_out_factor * cube::animation_scene_interval).count();
    constexpr int fade_time_max = fade_time_min + 1250;

    milliseconds fade_in_time{rand(range{fade_time_min, fade_time_max})};
    auto const fade_in_resolution = static_cast<unsigned int>(fade_in_time / cube::animation_scene_interval);

    c.voxel = random_voxel();
    c.in_fader = std::make_unique<ease_in_bounce>(context(), easing_config{{0.0, 1.0}, fade_in_resolution, fade_in_time},
        [&c]() { c.out_fader->start(); });
    c.out_fader = std::make_unique<ease_out_sine>(context(), easing_config{{1.0, 0.0}, fade_in_resolution / fade_out_factor, fade_in_time / fade_out_factor},
        [this, &c]() { spawn_cloud(c); });
    c.in_fader->start();
}

} // End of namespace
