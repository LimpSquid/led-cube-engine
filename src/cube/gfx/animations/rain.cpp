#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/easing.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct droplet
{
    glm::dvec3 position;
    glm::dvec3 velocity;
    gradient hue;

    void paint(painter & p, color_vec_t fade = rgba_vec(1.0)) const;
    void move(std::chrono::milliseconds const & dt);
};

struct rain :
    configurable_animation
{
    rain(engine_context & context);

    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    droplet make_droplet() const;

    std::vector<droplet> droplets_;
    std::vector<color> droplet_colors_;
    ease_in_sine fader_;
};

animation_publisher<rain> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr glm::dvec3 gravity{0.0, 0.0, -0.000001 * cube::cube_size_1d}; // Traveled distance under gravity is one cube_size_1d per 2 seconds
constexpr unsigned int default_number_of_droplets{cube::cube_size_1d * 3};
constexpr double default_motion_blur{0.9};
std::vector<color> const default_droplet_colors{color_steel_blue, color_cyan};

rain::rain(engine_context & context) :
    configurable_animation(context),
    fader_(context, {{0.1, 1.0}, 50, 1000ms})
{ }

void rain::state_changed(animation_state state)
{
    switch (state) {
        case animation_state::running: {
            droplet_colors_ = read_property<std::vector<color>>("droplet_colors");

            auto const num_droplets = read_property<unsigned int>("number_of_droplets");
            droplets_.resize(num_droplets);
            for (auto & droplet : droplets_) {
                droplet = make_droplet();
                droplet.position.z = rand(cube_axis_range);
            }

            fader_.start();
            break;
        }
        case animation_state::stopped:
            fader_.stop();
            break;
        default:;
    }
}

void rain::scene_tick(milliseconds dt)
{
    for (auto & droplet : droplets_) {
        droplet.move(dt);
        if (!visible(droplet.position))
            droplet = make_droplet();
    }
}

void rain::paint(graphics_device & device)
{
    painter p(device);

    for (auto const & droplet : droplets_)
        droplet.paint(p, rgb_vec(fader_.value()));
}

std::unordered_map<std::string, property_value_t> rain::extra_properties() const
{
    return {
        {"number_of_droplets", default_number_of_droplets},
        {"droplet_colors", default_droplet_colors},
        {"motion_blur", default_motion_blur},
    };
}

droplet rain::make_droplet() const
{
    auto const color = droplet_colors_.at(rand(range{std::size_t(0), droplet_colors_.size() - 1}));

    droplet droplet;
    droplet.position = random_voxel();
    droplet.position.z = cube::cube_axis_max_value; // Must start on top
    droplet.velocity = gravity * randd({0.0, 2000.0}); // TODO: make configurable?
    droplet.hue =
    {
        {1.0, adjust_brightness(color, randd({0.1, 0.6}))},
        {0.0, darker(color, 0.2)},
    };

    return droplet;
}

void droplet::paint(painter & p, color_vec_t fade) const
{
    p.set_color(hue(map(static_cast<int>(position.z), cube_axis_range, gradient_pos_range)).vec() * fade);
    p.draw(position);
}

void droplet::move(std::chrono::milliseconds const & dt)
{
    velocity += gravity * static_cast<double>(dt.count());
    position += velocity * static_cast<double>(dt.count());
}

} // End of namespace
