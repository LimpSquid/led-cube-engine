#include <cube/gfx/configurable_animation.hpp>
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

struct ball
{
    int radius;
    double mass;
    glm::dvec3 position;
    glm::dvec3 velocity;
    color c;

    void move(std::chrono::milliseconds const & dt);
};

struct falling_balls :
    configurable_animation
{
    falling_balls(engine_context & context);

    animation_trait traits() const override { return animation_trait::transition; }
    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    ball make_ball() const;

    std::vector<ball> balls_;
    std::vector<color> ball_colors_;
    std::optional<ease_linear> grow_in_;
    std::optional<ease_linear> shrink_out_;
    int max_radius_;
    int min_radius_;
};

animation_publisher<falling_balls> const publisher;

constexpr double gravity = -0.000002 * cube::cube_size_1d; // Traveled distance under gravity is one cube_size_1d per second
constexpr double default_motion_blur{0.75};
constexpr glm::dvec3 force{0.0, 0.0, gravity};
constexpr unsigned int default_number_of_balls{3};
constexpr int default_max_radius{cube::cube_size_1d / 4};
constexpr int default_min_radius{cube::cube_size_1d / 8};

falling_balls::falling_balls(engine_context & context) :
    configurable_animation(context)
{ }

void falling_balls::state_changed(animation_state state)
{
    switch (state) {
        case animation_state::running: {
            max_radius_ = read_property<int>("max_ball_radius");
            min_radius_ = read_property<int>("min_ball_radius");
            ball_colors_ = read_property<std::vector<color>>("ball_colors");

            auto const num_balls = read_property<unsigned int>("number_of_balls");
            balls_.resize(num_balls);
            for (auto & ball : balls_)
                ball = make_ball();

            grow_in_.emplace(context(), easing_config{{0.0, 1.0}, 50, get_transition_time()});
            shrink_out_.emplace(context(), easing_config{{1.0, 0.0}, 50, get_transition_time()});

            grow_in_->start();
            break;
        }
        case animation_state::stopping:
            shrink_out_->start();
            break;
        case animation_state::stopped:
            grow_in_->stop();
            shrink_out_->stop();
            break;
        default:;
    }
}

void falling_balls::scene_tick(milliseconds dt)
{
    for (auto & ball : balls_) {
        int const top = static_cast<int>(ball.position.z) + ball.radius;
        if (top >= 0)
            ball.move(dt);
        else
            ball = make_ball();
    }
}

void falling_balls::paint(graphics_device & device)
{
    painter p(device);
    p.set_fill_mode(graphics_fill_mode::none);

    double const grow_scalar = grow_in_->value() * shrink_out_->value();

    for (auto const & ball : balls_) {
        p.set_color(ball.c);
        p.sphere(ball.position, static_cast<int>(std::round(grow_scalar * ball.radius)));
    }
}

std::unordered_map<std::string, property_value_t> falling_balls::extra_properties() const
{
    return {
        {"number_of_balls", default_number_of_balls},
        {"max_ball_radius", default_max_radius},
        {"min_ball_radius", default_min_radius},
        {"ball_colors", std::vector<color>{}},
        {"motion_blur", default_motion_blur},
    };
}

ball falling_balls::make_ball() const
{
    auto const radius = map(randd(), randd_range, range(min_radius_, max_radius_));
    auto const mass = map(radius, range(min_radius_, max_radius_), range(2.0, 8.0));
    glm::dvec3 position = random_voxel();
    position.z = cube::cube_size_1d + map(randd(), randd_range, range(radius, 4 * radius)); // Spawn outside cube

    auto const color = ball_colors_.empty()
        ? random_color()
        : ball_colors_.at(rand(range{std::size_t(0), ball_colors_.size() - 1}));

    return {radius, mass, position, {}, color};
}

void ball::move(milliseconds const & dt)
{
    velocity += (force / mass) * static_cast<double>(dt.count());
    position += velocity * static_cast<double>(dt.count());
}

} // End of namespace
