#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/timers.hpp>
#include <cube/core/json_util.hpp>
#include <cube/gfx/animations/double_sine_wave.hpp>
#include <cube/gfx/animations/stars.hpp>
#include <cube/gfx/animations/helix.hpp>
#include <hal/graphics_device.hpp>
#include <chrono>
#include <iostream>

using namespace cube::core;
using namespace std::chrono;
namespace animations = cube::gfx::animations;

int main(int argc, char *argv[])
{
    engine_context context;
    engine cube_engine(context, graphics_device_factory<hal::graphics_device_t>{});

    animations::double_sine_wave double_sine_wave(context);
    animations::stars stars(context);
    animations::helix helix(context);
    animations::helix fat_helix(context);
    animations::helix long_helix(context);

    nlohmann::json j = {
        {"helix_rotation_time_ms", 1250},
        {"helix_phase_shift_sin_factor", 0.02},
        {"helix_phase_shift_cos_factor", 0.01},
        {"color_gradient_start", to_json(color_cyan)},
        {"color_gradient_end", to_json(color_yellow)},
    };
    helix.load_properties(j);

    fat_helix.write_properties({
        {animations::helix::helix_phase_shift_sin_factor, 0.02},
        {animations::helix::helix_phase_shift_cos_factor, 0.1},
        {animations::helix::helix_thickness, 10.0},
        {animations::helix::color_gradient_start, color_red},
        {animations::helix::color_gradient_end, color_orange},
    });

    long_helix.write_properties({
        {animations::helix::helix_phase_shift_sin_factor, 0.04},
        {animations::helix::color_gradient_start, color_magenta},
        {animations::helix::color_gradient_end, color_orange},
        {animations::helix::helix_length, 4.0},
        {animations::helix::helix_thickness, 1.5},
    });

    std::vector<animation *> animations = {
        &helix,
        &stars,
        &fat_helix,
        &long_helix,
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
