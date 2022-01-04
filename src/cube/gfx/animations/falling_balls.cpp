#include <cube/gfx/animations/falling_balls.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>
#include <cube/core/json_util.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::falling_balls> const publisher{"falling_balls"};

constexpr double gravity = -0.000002 * cube::cube_size_1d; // Traveled distance under gravity is one cube_size_1d per second
constexpr glm::dvec3 force{0.0, 0.0, gravity};
constexpr int default_number_of_balls{3};
constexpr int default_max_size{cube::cube_size_1d / 4};
constexpr int default_min_size{cube::cube_size_1d / 8};

} // End of namespace

namespace cube::gfx::animations
{

falling_balls::falling_balls(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this](auto elapsed) {
        for (auto & ball : balls_) {
            int const top = static_cast<int>(ball.position.z) + ball.size;
            if (top >= 0)
                ball.move(elapsed);
            else
                ball = make_ball();
        }
    })
{ }

void falling_balls::start()
{
    max_size_ = read_property(max_ball_size, default_max_size);
    min_size_ = read_property(min_ball_size, default_min_size);

    int num_balls = read_property(number_of_balls, default_number_of_balls);
    for (int i = 0; i < num_balls; ++i)
        balls_.push_back(make_ball());
    scene_.start();
}

void falling_balls::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();
    p.set_color(color_blue);
    p.set_fill_mode(graphics_fill_mode::none);

    for (auto const & ball : balls_)
        p.sphere(ball.position, ball.size);
}

void falling_balls::stop()
{
    scene_.stop();
}

nlohmann::json falling_balls::properties_to_json() const
{
    return {
        make_field(number_of_balls, read_property(number_of_balls, default_number_of_balls)),
        make_field(max_ball_size, read_property(max_ball_size, default_max_size)),
        make_field(min_ball_size, read_property(min_ball_size, default_max_size)),
    };
}

std::vector<falling_balls::property_pair_t> falling_balls::properties_from_json(nlohmann::json const & json) const
{
    return {
        {number_of_balls, parse_field(json, number_of_balls, default_number_of_balls)},
        {max_ball_size, parse_field(json, max_ball_size, default_max_size)},
        {min_ball_size, parse_field(json, min_ball_size, default_min_size)},
    };
}

falling_balls::ball falling_balls::make_ball() const
{
    auto const size = map(drand(), drand_range, range(min_size_, max_size_));
    auto const mass = map(size, range(min_size_, max_size_), range(0.1, 0.3));
    glm::dvec3 position = random_voxel();
    position.z = cube_size_1d + map(drand(), drand_range, range(size, 4 * size)); // Spawn outside cube

    return {size, mass, position, {}};
}

void falling_balls::ball::move(milliseconds const & dt)
{
    velocity += force * mass * static_cast<double>(dt.count());
    position += velocity * static_cast<double>(dt.count());
}

} // End of namespace
