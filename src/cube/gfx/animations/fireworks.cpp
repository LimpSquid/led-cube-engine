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

    fireworks(engine_context & context);

    void start() override;
    void paint(graphics_device & device) override;
    void stop() override;
    nlohmann::json properties_to_json() const override;
    std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    shell make_shell() const;

    std::vector<shell> shells_;
    std::vector<color> shell_colors_;
    animation_scene scene_;
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
            if (less_than(shell.velocity.z, 0.01) || !visible(shell.position))
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
    color const explosion_color = shell.hue(map(static_cast<int>(shell.position.z), cube_axis_range, gradient_pos_range)); // Get current color of shell

    for (auto & fragment : fragments) {
        color const c = adjust_brightness(explosion_color, randd());

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
            {0.00, color_transparent},
            {0.10, c},
            {1.00, c},
        };
    }

    state = exploded;
}

} // End of namespace
