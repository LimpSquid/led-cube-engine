#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/logging.hpp>

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
    std::optional<color> fade_color;
};

struct stars :
    configurable_animation
{
    stars(engine_context & context);

    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    color galaxy_color(voxel_t voxel) const;
    star make_star();

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

void stars::state_changed(animation_state state)
{
    switch (state) {
        case running: {
            auto fade_time = read_property<milliseconds>("fade_time_ms");
            if (fade_time < cube::animation_scene_interval) {
                LOG_WRN("Ignoring property 'fade_time_ms', must be atleast 'animation_scene_interval.",
                    LOG_ARG("animation_scene_interval", cube::animation_scene_interval));
                fade_time = cube::animation_scene_interval;
            }

            galaxy_gradient_ = read_property<gradient>("galaxy_gradient");
            star_radius_ = read_property<int>("star_radius");
            step_interval_ = static_cast<int>(fade_time / cube::animation_scene_interval);
            omega_ = M_PI / step_interval_; // omega = 0.5 * ((2 * pi) / step_interval_), multiply by 0.5 as we only use half a sine period for fading
            omega_gradient_ = omega_ * gradient_omega_scalar;
            gradient_step_ = 0;

            auto const num_stars = read_property<unsigned int>("number_of_stars");
            stars_.resize(num_stars);
            for (auto & star : stars_) {
                star = make_star();
                star.fade_step = -rand(range{0, step_interval_}); // Negative so stars are initially black
            }
            break;
        }
        default:;
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
        gradient fade({
            {0.0, color_transparent},
            {1.0, s.fade_color ? *s.fade_color : galaxy_color(s.voxel)},
        });

        p.set_color(fade(std::sin(s.fade_step * omega_))); // Half of sine period is used for fading the star, the other half the star is black
        p.sphere(s.voxel, star_radius_);
    }
}

std::unordered_map<std::string, property_value_t> stars::extra_properties() const
{
    return {
        { "fade_time_ms", default_fade_time },
        { "number_of_stars", default_number_of_stars },
        { "galaxy_gradient", default_galaxy_gradient },
        { "star_radius", default_radius },
    };
}

color stars::galaxy_color(voxel_t voxel) const
{
    double phase_shift = gradient_phase_shift_scalar * M_PI * (static_cast<double>(voxel.z) / cube::cube_size_1d);
    return galaxy_gradient_(abs_cos(gradient_step_ * omega_gradient_ - phase_shift));
}

star stars::make_star()
{
    voxel_t voxel = random_voxel();
    auto search = std::find_if(stars_.begin(), stars_.end(),
        [&](star const & s) { return s.voxel == voxel; });

    if (search == stars_.end())
        return {voxel, 0, {}};

    // If the new star is spawned on a location where another star
    // already lives lock that stars color to its current value.
    if (!search->fade_color)
        search->fade_color = galaxy_color(voxel);
    return {voxel, 0, color_transparent};
}

} // End of namespace
