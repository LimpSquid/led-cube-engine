#include <cube/gfx/animations/stars.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/gradient.hpp>
#include <cube/specs.hpp>

using namespace cube::core;
using namespace std::chrono;

namespace
{

constexpr double hue_omega_scalar = 0.125;
constexpr double hue_phase_shift_scalar = 0.1;
const gradient hue =
{
    {0.00, color_red    },
    {0.25, color_magenta},
    {0.50, color_orange },
    {0.75, color_yellow },
    {1.00, color_cyan   },
};

} // End of namespace

namespace cube::gfx::animations
{

stars::stars(engine_context & context) :
    animation_track(context)
{ }

void stars::configure()
{
    fade_resolution_ = std::max(1, read_property(fade_resolution, 50));
    omega_ = M_PI / fade_resolution_; // omega = 0.5 * ((2 * pi) / resolution), multiply by 0.5 as we only use half a sine period for fading
    hue_step_ = 0;

    int number_of_stars = read_property(number_of_stars, cube_size_3d / 15);
    stars_.resize(std::min(cube_size_3d / 8, number_of_stars)); // Max number of stars is 1/8th of the cube's size
    for (star & s : stars_) {
        s = make_unique_star();
        s.fade_step = -(std::rand() % fade_resolution_); // Negative so stars are initially black
    }

    tick_sub_ = tick_subscription::create(
        context(),
        read_property(fade_time_ms, 5000ms) / fade_resolution_,
        [this](auto, auto) {
            for (star & s : stars_)
                if (++s.fade_step > fade_resolution_)
                    s = make_unique_star();

            hue_step_++;
            update();
        }
    );
}

void stars::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    for (star const & s : stars_) {
        double phase_shift = hue_phase_shift_scalar * M_PI * (static_cast<double>(s.voxel.z) / cube_size_1d);
        double hue_omega = omega_ * hue_omega_scalar;
        gradient fade({
            {0.0, color_black},
            {1.0, hue(std::fabs(std::cos(hue_step_ * hue_omega - phase_shift)))},
        });

        p.set_color(fade(std::sin(s.fade_step * omega_))); // Half of sine period is used for fading the star, the other half the star is black
        p.draw(s.voxel);
    }
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
