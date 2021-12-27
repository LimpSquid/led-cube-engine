#include <cube/gfx/animations/stars.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/gradient.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/json_util.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

char const * const animation_name = "stars";
library_publisher<animations::stars> const library_pub = {animation_name};

constexpr milliseconds default_fade_time = 5000ms;
constexpr int default_number_of_stars = cube::cube_size_3d / 15;

constexpr double hue_omega_scalar = 0.5;
constexpr double hue_phase_shift_scalar = 0.25;
gradient const hue =
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
    configurable_animation(context, animation_name),
    scene_(*this, [this]() {
        for (star & s : stars_)
            if (++s.fade_step > step_interval_)
                s = make_unique_star();
        hue_step_++;
    })
{ }

void stars::start()
{
    step_interval_ = read_property(fade_time_ms, default_fade_time) / animation_scene_interval;
    omega_ = M_PI / step_interval_; // omega = 0.5 * ((2 * pi) / step_interval_), multiply by 0.5 as we only use half a sine period for fading
    omega_hue_= omega_ * hue_omega_scalar;
    hue_step_ = 0;

    int number_of_stars = read_property(number_of_stars, default_number_of_stars);
    stars_.resize(std::min(cube_size_3d / 8, number_of_stars)); // Max number of stars is 1/8th of the cube's size
    for (star & s : stars_) {
        s = make_unique_star();
        s.fade_step = -(std::rand() % step_interval_); // Negative so stars are initially black
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

std::vector<stars::property_pair> stars::parse(nlohmann::json const & json) const
{
    return {
        {fade_time_ms, parse_field(json, fade_time_ms, default_fade_time)},
        {number_of_stars, parse_field(json, number_of_stars, default_number_of_stars)},
    };
}

stars::star stars::make_unique_star() const
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
