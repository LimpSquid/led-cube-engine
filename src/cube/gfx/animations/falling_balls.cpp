#include <cube/gfx/animations/falling_balls.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::falling_balls> const publisher;

constexpr double gravity = -0.000002 * cube::cube_size_1d; // Traveled distance under gravity is one cube_size_1d per second
constexpr glm::dvec3 force{0.0, 0.0, gravity};
constexpr unsigned int default_number_of_balls{3};
constexpr int default_max_radius{cube::cube_size_1d / 4};
constexpr int default_min_radius{cube::cube_size_1d / 8};

} // End of namespace

namespace cube::gfx::animations
{

falling_balls::falling_balls(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this](auto elapsed) {
        for (auto & ball : balls_) {
            int const top = static_cast<int>(ball.position.z) + ball.radius;
            if (top >= 0)
                ball.move(elapsed);
            else
                ball = make_ball();
        }
    })
{ }

void falling_balls::start()
{
    max_radius_ = read_property(max_ball_radius, default_max_radius);
    min_radius_ = read_property(min_ball_radius, default_min_radius);
    ball_colors_ = read_property(ball_colors, std::vector<color>{});

    unsigned int num_balls = read_property(number_of_balls, default_number_of_balls);
    balls_.resize(num_balls);
    for (auto & ball : balls_)
        ball = make_ball();

    scene_.start();
}

void falling_balls::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();
    p.set_fill_mode(graphics_fill_mode::none);

    for (auto const & ball : balls_) {
        p.set_color(ball.color);
        p.sphere(ball.position, ball.radius);
    }
}

void falling_balls::stop()
{
    scene_.stop();
}

nlohmann::json falling_balls::properties_to_json() const
{
    return {
        to_json(number_of_balls, default_number_of_balls),
        to_json(max_ball_radius, default_max_radius),
        to_json(min_ball_radius, default_min_radius),
        to_json(ball_colors, std::vector<color>{}),
    };
}

std::vector<falling_balls::property_pair_t> falling_balls::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, number_of_balls, default_number_of_balls),
        from_json(json, max_ball_radius, default_max_radius),
        from_json(json, min_ball_radius, default_min_radius),
        from_json(json, ball_colors, std::vector<color>{}),
    };
}

falling_balls::ball falling_balls::make_ball() const
{
    auto const radius = map(randd(), randd_range, range(min_radius_, max_radius_));
    auto const mass = map(radius, range(min_radius_, max_radius_), range(2.0, 8.0));
    glm::dvec3 position = random_voxel();
    position.z = cube_size_1d + map(randd(), randd_range, range(radius, 4 * radius)); // Spawn outside cube

    auto const color = ball_colors_.empty()
        ? random_color()
        : ball_colors_.at(rand(range{size_t(0), ball_colors_.size() - 1}));

    return {radius, mass, position, {}, color};
}

void falling_balls::ball::move(milliseconds const & dt)
{
    velocity += (force / mass) * static_cast<double>(dt.count());
    position += velocity * static_cast<double>(dt.count());
}

} // End of namespace
