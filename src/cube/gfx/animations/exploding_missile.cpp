#include <cube/gfx/animations/exploding_missile.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::exploding_missile> const publisher;

constexpr cube::core::range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr double gravity = -0.000001 * cube::cube_size_1d; // Traveled distance under gravity is one cube_size_1d per 2 seconds
constexpr glm::dvec3 force{0.0, 0.0, gravity};

} // End of namespace

namespace cube::gfx::animations
{

exploding_missile::exploding_missile(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this](auto elapsed) {
        for (auto & missile : missiles_) {
            missile.update(elapsed);
            if (missile.state == missile::completed)
                missile = make_missile();
        }
    })
{ }

void exploding_missile::start()
{
    missiles_.resize(3);
    for (auto & missile : missiles_)
        missile = make_missile();

    scene_.start();
}

void exploding_missile::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (auto const & missile : missiles_)
        missile.paint(p);
}

void exploding_missile::stop()
{
    scene_.stop();
}

nlohmann::json exploding_missile::properties_to_json() const
{
    return {

    };
}

std::vector<exploding_missile::property_pair_t> exploding_missile::properties_from_json(nlohmann::json const & json) const
{
    return {

    };
}

exploding_missile::missile exploding_missile::make_missile() const
{
    particle trail;
    trail.hue =
    {
        {0.0, random_color()},
        {1.0, color_white},
    };

    glm::dvec3 target = random_voxel();
    target.z = cube_axis_max_value; // Must end on top

    trail.position = random_voxel();
    trail.position.z = cube_axis_min_value; // Must start on bottom
    trail.velocity = (target - trail.position) * map(std::rand(), rand_range, range{0.001, 0.002}); // 1 - 2 secs

    return {std::move(trail), {}};
}

void exploding_missile::particle::move(milliseconds const & dt)
{
    velocity += force * static_cast<double>(dt.count());
    position += velocity * static_cast<double>(dt.count());
}

void exploding_missile::particle::paint(painter & p) const
{
    p.set_color(hue(map(static_cast<int>(position.z), cube_axis_range, gradient_pos_range)));
    p.draw(position);
}

void exploding_missile::missile::update(std::chrono::milliseconds const & dt)
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

void exploding_missile::missile::paint(painter & p) const
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

void exploding_missile::missile::explode()
{
    color const explosion_color = trail.hue(0.0); // Begin color of trail
    double const explosion_factor = map(std::rand(), rand_range, range{0.005, 0.04});

    fragments.resize(250);
    for (auto & fragment : fragments) {
        color const c = adjust_brightness(explosion_color, drand());

        fragment.position = trail.position;
        fragment.velocity.x = map(std::rand(), rand_range, unit_circle_range) * explosion_factor;
        fragment.velocity.y = map(std::rand(), rand_range, unit_circle_range) * explosion_factor;
        fragment.velocity.z = map(std::rand(), rand_range, unit_circle_range) * explosion_factor;
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
