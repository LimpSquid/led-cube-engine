#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/subscriptions.hpp>
#include <cube/hal/mock/display.hpp>
#include <cube/gfx/animations/fill_cube.hpp>
#include <cube/gfx/animations/double_sine_wave.hpp>
#include <cube/gfx/animations/stars.hpp>
#include <cube/gfx/animations/helix.hpp>
#include <chrono>

using namespace cube::core;
using namespace std::chrono;
namespace animations = cube::gfx::animations;
namespace mock = cube::hal::mock;

int main(int argc, char *argv[])
{
    engine_context context;
    engine cube_engine(context, new mock::display);

    animations::fill_cube fill_cube(context);
    animations::double_sine_wave double_sine_wave(context);
    animations::stars stars(context);
    animations::helix helix(context);

    std::vector<animation *> animations = {
        //&helix,
        &stars,
        // &fill_cube,
        &double_sine_wave,
    };
    int animations_index = 0;

    tick_subscription tick_sub(
        context,
        15s,
        [&](auto, auto) { cube_engine.load(animations[animations_index++ % animations.size()]); },
        true
    );
    cube_engine.run(); // Todo: eventually we need to cycle through animations

    return EXIT_SUCCESS;
}
