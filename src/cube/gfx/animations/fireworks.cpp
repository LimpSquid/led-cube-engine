#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/gradient.hpp>
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

struct paint_context
{
    painter & p;
    double fade;
};

struct particle
{
    int radius;
    glm::dvec3 position;
    glm::dvec3 velocity;
    gradient hue;

    void move(std::chrono::milliseconds const & dt);
    void paint(paint_context const & ctx) const;
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
    void paint(paint_context const & ctx) const;
    void explode();
};

struct fireworks :
    configurable_animation
{
    fireworks(engine_context & context);

    animation_trait traits() const override { return animation_trait::transition; }
    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    shell make_shell() const;

    std::vector<shell> shells_;
    std::vector<color> shell_colors_;
    std::optional<ease_in_sine> fade_in_;
    std::optional<ease_out_sine> fade_out_;
    double explosion_force_;
    unsigned int num_fragments_;
    int shell_radius_;
};

animation_publisher<fireworks> const publisher;

constexpr range cube_axis_range{cube::cube_axis_min_value, cube::cube_axis_max_value};
constexpr double default_explosion_force = 2.5;
constexpr double default_motion_blur = 0.95;
constexpr double gravity = -0.000001 * cube::cube_size_1d; // Traveled distance under gravity is one cube_size_1d per 2 seconds
constexpr glm::dvec3 force{0.0, 0.0, gravity};
constexpr unsigned int default_number_of_shells{3};
constexpr unsigned int default_number_of_fragments{cube::cube_size_1d * 15};
constexpr int default_shell_radius{cube::cube_size_1d / 8};

fireworks::fireworks(engine_context & context) :
    configurable_animation(context)
{ }

void fireworks::state_changed(animation_state state)
{
    switch (state) {
        case animation_state::running: {
            explosion_force_ = read_property<double>("explosion_force");
            shell_colors_ = read_property<std::vector<color>>("shell_colors");
            num_fragments_ = read_property<unsigned int>("number_of_fragments");
            shell_radius_ = read_property<int>("shell_radius");

            auto const num_shells = read_property<unsigned int>("number_of_shells");
            shells_.resize(num_shells);
            for (auto & shell : shells_)
                shell = make_shell();

            fade_in_.emplace(context(), easing_config{{0.0, 1.0}, 50, get_transition_time()});
            fade_out_.emplace(context(), easing_config{{1.0, 0.0}, 50, get_transition_time()});

            fade_in_->start();
            break;
        }
        case animation_state::stopping:
            fade_out_->start();
            break;
        case animation_state::stopped:
            fade_in_->stop();
            fade_out_->stop();
            break;
    }
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

    paint_context ctx{p, fade_in_->value() * fade_out_->value()};
    for (auto const & shell : shells_)
        shell.paint(ctx);
}

std::unordered_map<std::string, property_value_t> fireworks::extra_properties() const
{
    return {
        {"number_of_shells", default_number_of_shells},
        {"number_of_fragments", default_number_of_fragments},
        {"shell_radius", default_shell_radius},
        {"explosion_force", default_explosion_force},
        {"shell_colors", std::vector<color>{}},
        {"motion_blur", default_motion_blur},
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

void particle::paint(paint_context const & ctx) const
{
    auto c = hue(map(static_cast<int>(position.z), cube_axis_range, gradient_pos_range)).vec();
    c *= rgb_vec(ctx.fade);

    ctx.p.set_color(c);
    ctx.p.sphere(position, radius);
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

void shell::paint(paint_context const & ctx) const
{
    switch (state) {
        case flying:
            shell.paint(ctx);
            break;
        case exploded:
            for (auto const & fragment : fragments)
                fragment.paint(ctx);
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
