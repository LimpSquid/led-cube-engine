#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct particle
{
    int radius;
    glm::dvec3 position;
    glm::dvec3 velocity;
    gradient hue;

    void move(std::chrono::milliseconds const & dt);
    void paint(painter & p) const;
};

struct shell
{
    enum state
    {
        flying,
        exploded,
        completed,
    };

    particle shell;
    std::vector<particle> fragments;
    double explosion_force;
    state state{flying};

    void update(std::chrono::milliseconds const & dt);
    void paint(painter & p) const;
    void explode();
};

struct fireworks :
    configurable_animation
{
    PROPERTY_ENUM
    (
        number_of_shells,       // Number of shells
        number_of_fragments,    // Number of fragments when exploded
        explosion_force,        // Factor to limit or increase the explosion force
        shell_radius,           // Radius of the shell
        shell_colors,           // Array of shell colors to pick from
    )

    fireworks(engine_context & context);

    void start() override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    json_or_error_t properties_to_json() const override;
    property_pairs_or_error_t properties_from_json(nlohmann::json const & json) const override;

    shell make_shell() const;

    std::vector<shell> shells_;
    std::vector<color> shell_colors_;
    double explosion_force_;
    unsigned int num_fragments_;
    int shell_radius_;
};

animation_publisher<fireworks> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr double default_explosion_force = 1.0;
constexpr double gravity = -0.000001 * cube::cube_size_1d; // Traveled distance under gravity is one cube_size_1d per 2 seconds
constexpr glm::dvec3 force{0.0, 0.0, gravity};
constexpr unsigned int default_number_of_shells{3};
constexpr unsigned int default_number_of_fragments{cube::cube_size_1d * 15};
constexpr int default_shell_radius{cube::cube_size_1d / 8};

fireworks::fireworks(engine_context & context) :
    configurable_animation(context)
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
}

void fireworks::scene_tick(milliseconds dt)
{
    for (auto & shell : shells_) {
        shell.update(dt);
        if (shell.state == shell::completed)
            shell = make_shell();
    }
}

void fireworks::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (auto const & shell : shells_)
        shell.paint(p);
}

json_or_error_t fireworks::properties_to_json() const
{
    return nlohmann::json {
        make_json(number_of_shells, default_number_of_shells),
        make_json(number_of_fragments, default_number_of_fragments),
        make_json(shell_radius, default_shell_radius),
        make_json(explosion_force, default_explosion_force),
        make_json(shell_colors, std::vector<color>{}),
    };
}

property_pairs_or_error_t fireworks::properties_from_json(nlohmann::json const & json) const
{
    return property_pairs_t {
        make_property(json, number_of_shells, default_number_of_shells),
        make_property(json, number_of_fragments, default_number_of_fragments),
        make_property(json, shell_radius, default_shell_radius),
        make_property(json, explosion_force, default_explosion_force),
        make_property(json, shell_colors, std::vector<color>{}),
    };
}

shell fireworks::make_shell() const
{
    auto const pick_color = [this]() {
        return shell_colors_.empty()
            ? random_color()
            : shell_colors_.at(rand(range{std::size_t(0), shell_colors_.size() - 1}));
    };

    glm::dvec3 target = random_voxel();
    target.z = cube::cube_axis_max_value; // Must end on top

    particle shell;
    shell.radius = shell_radius_;
    shell.position = random_voxel();
    shell.position.z = cube::cube_axis_min_value; // Must start on bottom
    shell.velocity = (target - shell.position) * randd({0.001, 0.002});
    shell.hue =
    {
        {0.0, pick_color()},
        {1.0, pick_color()},
    };

    return {std::move(shell), std::vector<particle>(num_fragments_), explosion_force_};
}

void particle::move(milliseconds const & dt)
{
    velocity += force * static_cast<double>(dt.count());
    position += velocity * static_cast<double>(dt.count());
}

void particle::paint(painter & p) const
{
    p.set_color(hue(map(static_cast<int>(position.z), cube_axis_range, gradient_pos_range)));
    p.sphere(position, radius);
}

void shell::update(std::chrono::milliseconds const & dt)
{
    switch (state) {
        case flying:
            shell.move(dt);
            if (less_than(shell.velocity.z, 0.01) || !visible(shell.position))
                explode();
            break;
        case exploded: {
            bool done = true;
            for (auto & fragment : fragments) {
                fragment.move(dt);
                done &= !visible(fragment.position);
            }
            if (done)
                state = completed;
            break;
        }
        default:;
    }
}

void shell::paint(painter & p) const
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

void shell::explode()
{
    color const explosion_color = shell.hue(map(static_cast<int>(shell.position.z), cube_axis_range, gradient_pos_range)); // Get current color of shell

    for (auto & fragment : fragments) {
        color const c = adjust_brightness(explosion_color, randd({0.1, 0.6}));

        // Use polar coordinate system for picking random points inside a sphere
        // https://datagenetics.com/blog/january32020/index.html
        double const theta = randd({0.0, 2 * M_PI});
        double const phi = std::acos(2.0 * randd() - 1.0);
        double const radius = shell.radius * std::pow(randd(), 1.0 / 3.0);
        glm::dvec3 const fragment_offset =
        {
            radius * std::sin(phi) * std::cos(theta),
            radius * std::sin(phi) * std::sin(theta),
            radius * std::cos(phi)
        };

        fragment.radius = 0;
        fragment.position = shell.position + fragment_offset;

        // If it's exploding because of gravity pulling it down use the shell's velocity for all
        // the fragments so that they're moving in the same direction as the shell was. If the
        // shell hits a wall, then we start with no initial velocity so that the particles will
        // not be moving to the outside of the cube where they are not visible.
        fragment.velocity = visible(shell.position) ? shell.velocity : glm::dvec3{};
        fragment.velocity.x += randd({-0.02, 0.02}) * explosion_force;
        fragment.velocity.y += randd({-0.02, 0.02}) * explosion_force;
        // Limit max upwards velocity, otherwise it can take a long time until all particles fall back to earth
        double const vz = randd({-0.02, 0.02}) * explosion_force;
        fragment.velocity.z += less_than_or_equal(vz, 0.06) ? vz : randd({0.02, 0.06});
        fragment.hue =
        {
            {0.00, color_white},
            {0.15, c},
            {1.00, c},
        };
    }

    state = exploded;
}

} // End of namespace
