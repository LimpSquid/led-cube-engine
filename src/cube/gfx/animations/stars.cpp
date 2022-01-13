#include <cube/gfx/animations/stars.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

animation_publisher<animations::stars> const publisher;

constexpr milliseconds default_fade_time{5000ms};
constexpr int default_number_of_stars{cube::cube_size_3d / 15};

constexpr double hue_omega_scalar{0.5};
constexpr double hue_phase_shift_scalar{0.25};
gradient const hue
{
    {0.00, color_red},
    {0.25, color_cyan},
    {0.50, color_magenta},
    {0.75, color_yellow},
    {1.00, color_orange},
};

} // End of namespace

namespace cube::gfx::animations
{

stars::stars(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this](auto) {
        for (auto & star : stars_)
            if (++star.fade_step > step_interval_)
                star = make_star();
        hue_step_++;
    })
{ }

void stars::start()
{
    step_interval_ = static_cast<int>(read_property(fade_time_ms, default_fade_time) / animation_scene_interval);
    omega_ = M_PI / step_interval_; // omega = 0.5 * ((2 * pi) / step_interval_), multiply by 0.5 as we only use half a sine period for fading
    omega_hue_= omega_ * hue_omega_scalar;
    hue_step_ = 0;

    int num_stars = std::min(cube_size_3d / 8, read_property(number_of_stars, default_number_of_stars));
    stars_.resize(num_stars);
    for (auto & star : stars_) {
        star = make_star();
        star.fade_step = -(std::rand() % step_interval_); // Negative so stars are initially black
    }

    scene_.start();
}

void stars::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (star const & s : stars_) {
        double phase_shift = hue_phase_shift_scalar * M_PI * (static_cast<double>(s.voxel.z) / cube_size_1d);
        gradient fade({
            {0.0, color_black},
            {1.0, hue(abs_cos(hue_step_ * omega_hue_ - phase_shift))},
        });

        p.set_color(fade(std::sin(s.fade_step * omega_))); // Half of sine period is used for fading the star, the other half the star is black
        p.draw(s.voxel);
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
    };
}

std::vector<stars::property_pair_t> stars::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, fade_time_ms, default_fade_time),
        from_json(json, number_of_stars, default_number_of_stars),
    };
}

stars::star stars::make_star() const
{
    std::vector<star>::const_iterator search;
    voxel_t voxel;

    do {
        voxel = random_voxel();
        search = std::find_if(stars_.begin(), stars_.end(),
            [&](star const & s) { return s.voxel == voxel; });
    } while(search != stars_.end()); // Safe as we never have more stars than 1/8th of the cube's size

    return {voxel, 0};
}

} // End of namespace
