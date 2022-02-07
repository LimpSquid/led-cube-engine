#include <cube/gfx/animations/fireworks.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::fireworks> const publisher;

constexpr cube::core::range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr double default_explosion_force = 1.0;
constexpr double gravity = -0.000001 * cube::cube_size_1d; // Traveled distance under gravity is one cube_size_1d per 2 seconds
constexpr glm::dvec3 force{0.0, 0.0, gravity};
constexpr unsigned int default_number_of_rockets{3};
constexpr unsigned int default_number_of_fragments{cube::cube_size_1d * 15};
constexpr int default_trail_radius{cube::cube_size_1d / 8};

} // End of namespace

namespace cube::gfx::animations
{

fireworks::fireworks(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this](auto elapsed) {
        for (auto & rocket : rockets_) {
            rocket.update(elapsed);
            if (rocket.state == rocket::completed)
                rocket = make_rocket();
        }
    })
{ }

void fireworks::start()
{
    explosion_force_ = read_property(explosion_force, default_explosion_force);
    rocket_colors_ = read_property(rocket_colors, std::vector<color>{});
    num_fragments_ = read_property(number_of_fragments, default_number_of_fragments);
    trail_radius_ = read_property(rocket_trail_radius, default_trail_radius);

    unsigned int num_rockets = read_property(number_of_rockets, default_number_of_rockets);
    rockets_.resize(num_rockets);
    for (auto & rocket : rockets_)
        rocket = make_rocket();

    scene_.start();
}

void fireworks::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (auto const & rocket : rockets_)
        rocket.paint(p);
}

void fireworks::stop()
{
    scene_.stop();
}

nlohmann::json fireworks::properties_to_json() const
{
    return {
        to_json(number_of_rockets, default_number_of_rockets),
        to_json(number_of_fragments, default_number_of_fragments),
        to_json(rocket_trail_radius, default_trail_radius),
        to_json(explosion_force, default_explosion_force),
        to_json(rocket_colors, std::vector<color>{}),
    };
}

std::vector<fireworks::property_pair_t> fireworks::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, number_of_rockets, default_number_of_rockets),
        from_json(json, number_of_fragments, default_number_of_fragments),
        from_json(json, rocket_trail_radius, default_trail_radius),
        from_json(json, explosion_force, default_explosion_force),
        from_json(json, rocket_colors, std::vector<color>{}),
    };
}

fireworks::rocket fireworks::make_rocket() const
{
    auto const pick_color = [this]() {
        return rocket_colors_.empty()
            ? random_color()
            : rocket_colors_.at(urand() % rocket_colors_.size());
    };

    particle trail;
    glm::dvec3 target = random_voxel();
    target.z = cube_axis_max_value; // Must end on top

    trail.radius = trail_radius_;
    trail.position = random_voxel();
    trail.position.z = cube_axis_min_value; // Must start on bottom
    trail.velocity = (target - trail.position) * map(std::rand(), rand_range, range{0.001, 0.002});
    trail.hue =
    {
        {0.0, pick_color()},
        {1.0, pick_color()},
    };

    return {std::move(trail), std::vector<particle>(num_fragments_), explosion_force_};
}

void fireworks::particle::move(milliseconds const & dt)
{
    velocity += force * static_cast<double>(dt.count());
    position += velocity * static_cast<double>(dt.count());
}

void fireworks::particle::paint(painter & p) const
{
    p.set_color(hue(map(static_cast<int>(position.z), cube_axis_range, gradient_pos_range)));
    p.sphere(position, radius);
}

void fireworks::rocket::update(std::chrono::milliseconds const & dt)
{
    switch (state) {
        case flying:
            trail.move(dt);
            if (less_than(trail.velocity.z, 0.0) || !visible(trail.position))
                explode();
            break;
        case exploded: {
            bool x = true;
            for (auto & fragment : fragments) {
                fragment.move(dt);
                x &= less_than(fragment.position.z, 0.0);
            }
            if (x)
                state = completed;
            break;
        }
        default:;
    }
}

void fireworks::rocket::paint(painter & p) const
{
    switch (state) {
        case flying:
            trail.paint(p);
            break;
        case exploded:
            for (auto const & fragment : fragments)
                fragment.paint(p);
            break;
        default:;
    }
}

void fireworks::rocket::explode()
{
    color const explosion_color = trail.hue(map(static_cast<int>(trail.position.z), cube_axis_range, gradient_pos_range)); // Get current color of trail
    double const explosion_factor = map(std::rand(), rand_range, range{0.05, 0.02}) * explosion_force;

    for (auto & fragment : fragments) {
        color const c = adjust_brightness(explosion_color, drand());

        fragment.radius = 0;
        fragment.position = trail.position;
        fragment.velocity.x = map(std::rand(), rand_range, unit_circle_range) * explosion_factor;
        fragment.velocity.y = map(std::rand(), rand_range, unit_circle_range) * explosion_factor;
        fragment.velocity.z = map(std::rand(), rand_range,  range{-1.0, 0.4}) * explosion_factor; // Limit upward velocity
        fragment.hue =
        {
            {0.00, color_transparent},
            {0.10, c},
            {1.00, c},
        };
    }

    state = exploded;
}

} // End of namespace
