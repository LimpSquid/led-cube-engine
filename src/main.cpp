#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/timers.hpp>
#include <cube/gfx/animations/fill_cube.hpp>
#include <cube/gfx/animations/double_sine_wave.hpp>
#include <cube/gfx/animations/stars.hpp>
#include <cube/gfx/animations/helix.hpp>
#include <hal/target.hpp>
#include <chrono>

using namespace cube::core;
using namespace std::chrono;
namespace animations = cube::gfx::animations;

int main(int argc, char *argv[])
{
    engine_context context;
    engine cube_engine(context, new hal::graphics_device_t);

    animations::fill_cube fill_cube(context);
    animations::double_sine_wave double_sine_wave(context);
    animations::stars stars(context);
    animations::helix helix(context);
    animations::helix fat_helix(context);

    helix.write_properties({
        {animations::helix::helix_phase_shift_sin_factor, 0.02},
        {animations::helix::helix_phase_shift_cos_factor, 0.1},
        {animations::helix::color_gradient_start, color_cyan},
        {animations::helix::color_gradient_end, color_yellow},
    });

    fat_helix.write_properties({
        {animations::helix::helix_phase_shift_sin_factor, 0.02},
        {animations::helix::helix_phase_shift_cos_factor, 0.1},
        {animations::helix::helix_thickness, 10.0},
        {animations::helix::color_gradient_start, color_red},
        {animations::helix::color_gradient_end, color_orange},
    });

    std::vector<animation *> animations = {
        &helix,
        &stars,
        &fat_helix,
        // &fill_cube,
        &double_sine_wave,
    };
    int animations_index = 0;

    recurring_timer timer(context, [&](auto, auto) {
        cube_engine.load(animations[animations_index++ % animations.size()]);
    });
    timer.start(20s, true);
    cube_engine.run(); // Todo: eventually we need to cycle through animations

    return EXIT_SUCCESS;
}
