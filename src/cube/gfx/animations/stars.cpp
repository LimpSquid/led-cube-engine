#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/painter.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

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
    void paint(graphics_device & device) override;
    void stop() override;
    nlohmann::json properties_to_json() const override;
    std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override;

    star make_star() const;

    std::vector<star> stars_;
    animation_scene scene_;
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
    configurable_animation(context),
    scene_(*this, [this](auto) {
        for (auto & star : stars_)
            if (++star.fade_step > step_interval_)
                star = make_star();
        gradient_step_++;
    })
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

    scene_.start();
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

void stars::stop()
{
    scene_.stop();
}

nlohmann::json stars::properties_to_json() const
{
    return {
        to_json(fade_time_ms, default_fade_time),
        to_json(number_of_stars, default_number_of_stars),
        to_json(galaxy_gradient, default_galaxy_gradient),
        to_json(star_radius, default_radius),
    };
}

std::vector<stars::property_pair_t> stars::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, fade_time_ms, default_fade_time),
        from_json(json, number_of_stars, default_number_of_stars),
        from_json(json, galaxy_gradient, default_galaxy_gradient),
        from_json(json, star_radius, default_radius),
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
