#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/painter.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;
using std::operator""s;

namespace
{

struct star
{
    voxel_t voxel;
    int fade_step;
};

struct stars :
    configurable_animation
{
    PROPERTY_ENUM
    (
        number_of_stars,    // Number of unique stars in the cube
        fade_time_ms,       // Fade time of star
        galaxy_gradient,    // Gradient of the galaxiy
        star_radius,        // Radius of a star
    )

    stars(engine_context & context);

    void start() override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    json_or_error_t properties_to_json() const override;
    property_pairs_or_error_t properties_from_json(nlohmann::json const & json) const override;

    star make_star() const;

    std::vector<star> stars_;
    gradient galaxy_gradient_;
    int gradient_step_;
    int step_interval_;
    int star_radius_;
    double omega_;
    double omega_gradient_;
};

animation_publisher<stars> const publisher;

constexpr double gradient_omega_scalar{0.5};
constexpr double gradient_phase_shift_scalar{0.25};
constexpr milliseconds default_fade_time{5000ms};
constexpr unsigned int default_number_of_stars{static_cast<unsigned int>(cube::cube_size_3d) / 15};
constexpr int default_radius{0};
gradient const default_galaxy_gradient
{
    {0.00, color_red},
    {0.25, color_cyan},
    {0.50, color_magenta},
    {0.75, color_yellow},
    {1.00, color_orange},
};

stars::stars(engine_context & context) :
    configurable_animation(context)
{ }

void stars::start()
{
    galaxy_gradient_ = read_property(galaxy_gradient, default_galaxy_gradient);
    star_radius_ = read_property(star_radius, default_radius);
    step_interval_ = static_cast<int>(read_property(fade_time_ms, default_fade_time) / cube::animation_scene_interval);
    omega_ = M_PI / step_interval_; // omega = 0.5 * ((2 * pi) / step_interval_), multiply by 0.5 as we only use half a sine period for fading
    omega_gradient_ = omega_ * gradient_omega_scalar;
    gradient_step_ = 0;

    unsigned int num_stars = read_property(number_of_stars, default_number_of_stars);
    stars_.resize(num_stars);
    for (auto & star : stars_) {
        star = make_star();
        star.fade_step = -rand(range{0, step_interval_}); // Negative so stars are initially black
    }
}

void stars::scene_tick(milliseconds)
{
    for (auto & star : stars_)
        if (++star.fade_step > step_interval_)
            star = make_star();
    gradient_step_++;
}

void stars::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (star const & s : stars_) {
        double phase_shift = gradient_phase_shift_scalar * M_PI * (static_cast<double>(s.voxel.z) / cube::cube_size_1d);
        gradient fade({
            {0.0, color_transparent},
            {1.0, galaxy_gradient_(abs_cos(gradient_step_ * omega_gradient_ - phase_shift))},
        });

        p.set_color(fade(std::sin(s.fade_step * omega_))); // Half of sine period is used for fading the star, the other half the star is black
        p.sphere(s.voxel, star_radius_);
    }
}

json_or_error_t stars::properties_to_json() const
{
    return nlohmann::json {
        make_json(fade_time_ms, default_fade_time),
        make_json(number_of_stars, default_number_of_stars),
        make_json(galaxy_gradient, default_galaxy_gradient),
        make_json(star_radius, default_radius),
    };
}

property_pairs_or_error_t stars::properties_from_json(nlohmann::json const & json) const
{
    auto fade_time = parse_field(json, fade_time_ms, default_fade_time);
    if (fade_time < cube::animation_scene_interval)
        return unexpected_error{"Field '"s + to_string(fade_time_ms) + "' must be atleast "
            + std::to_string(cube::animation_scene_interval.count()) + "ms"};

    return property_pairs_t {
        make_property(fade_time_ms, std::move(fade_time)),
        make_property(json, number_of_stars, default_number_of_stars),
        make_property(json, galaxy_gradient, default_galaxy_gradient),
        make_property(json, star_radius, default_radius),
    };
}

star stars::make_star() const
{
    std::vector<star>::const_iterator search;
    voxel_t voxel;
    unsigned tries = 10;

    do {
        voxel = random_voxel();
        search = std::find_if(stars_.begin(), stars_.end(),
            [&](star const & s) { return s.voxel == voxel; });
    } while (--tries != 0 && search != stars_.end());

    return {voxel, 0};
}

} // End of namespace
