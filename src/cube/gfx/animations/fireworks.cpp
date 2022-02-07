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
constexpr unsigned int default_number_of_shells{3};
constexpr unsigned int default_number_of_fragments{cube::cube_size_1d * 15};
constexpr int default_shell_radius{cube::cube_size_1d / 8};

} // End of namespace

namespace cube::gfx::animations
{

fireworks::fireworks(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this](auto elapsed) {
        for (auto & shell : shells_) {
            shell.update(elapsed);
            if (shell.state == shell::completed)
                shell = make_shell();
        }
    })
{ }

void fireworks::start()
{
    explosion_force_ = read_property(explosion_force, default_explosion_force);
    shell_colors_ = read_property(shell_colors, std::vector<color>{});
    num_fragments_ = read_property(number_of_fragments, default_number_of_fragments);
    shell_radius_ = read_property(shell_radius, default_shell_radius);

    unsigned int num_shells = read_property(number_of_shells, default_number_of_shells);
    shells_.resize(num_shells);
    for (auto & shell : shells_)
        shell = make_shell();

    scene_.start();
}

void fireworks::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (auto const & shell : shells_)
        shell.paint(p);
}

void fireworks::stop()
{
    scene_.stop();
}

nlohmann::json fireworks::properties_to_json() const
{
    return {
        to_json(number_of_shells, default_number_of_shells),
        to_json(number_of_fragments, default_number_of_fragments),
        to_json(shell_radius, default_shell_radius),
        to_json(explosion_force, default_explosion_force),
        to_json(shell_colors, std::vector<color>{}),
    };
}

std::vector<fireworks::property_pair_t> fireworks::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, number_of_shells, default_number_of_shells),
        from_json(json, number_of_fragments, default_number_of_fragments),
        from_json(json, shell_radius, default_shell_radius),
        from_json(json, explosion_force, default_explosion_force),
        from_json(json, shell_colors, std::vector<color>{}),
    };
}

fireworks::shell fireworks::make_shell() const
{
    auto const pick_color = [this]() {
        return shell_colors_.empty()
            ? random_color()
            : shell_colors_.at(rand<unsigned>() % shell_colors_.size());
    };

    particle shell;
    glm::dvec3 target = random_voxel();
    target.z = cube_axis_max_value; // Must end on top

    shell.radius = shell_radius_;
    shell.position = random_voxel();
    shell.position.z = cube_axis_min_value; // Must start on bottom
    shell.velocity = (target - shell.position) * map(rand(), rand_range, range{0.001, 0.002});
    shell.hue =
    {
        {0.0, pick_color()},
        {1.0, pick_color()},
    };

    return {std::move(shell), std::vector<particle>(num_fragments_), explosion_force_};
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

void fireworks::shell::update(std::chrono::milliseconds const & dt)
{
    switch (state) {
        case flying:
            shell.move(dt);
            if (less_than(shell.velocity.z, 0.0) || !visible(shell.position))
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

void fireworks::shell::paint(painter & p) const
{
    switch (state) {
        case flying:
            shell.paint(p);
            break;
        case exploded:
            for (auto const & fragment : fragments)
                fragment.paint(p);
            break;
        default:;
    }
}

void fireworks::shell::explode()
{
    range const fragment_box{-shell.radius / 2.0, shell.radius / 2.0};
    color const explosion_color = shell.hue(map(static_cast<int>(shell.position.z), cube_axis_range, gradient_pos_range)); // Get current color of shell

    for (auto & fragment : fragments) {
        color const c = adjust_brightness(explosion_color, randd());

        fragment.radius = 0;
        fragment.position.x = shell.position.x + map(rand(), rand_range, fragment_box);
        fragment.position.y = shell.position.y + map(rand(), rand_range, fragment_box);
        fragment.position.z = shell.position.z + map(rand(), rand_range, fragment_box);
        fragment.velocity.x = map(rand(), rand_range, range{-0.02, 0.02}) * explosion_force;
        fragment.velocity.y = map(rand(), rand_range, range{-0.02, 0.02}) * explosion_force;
        fragment.velocity.z = map(rand(), rand_range, range{-0.02, 0.02}) * explosion_force;
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
