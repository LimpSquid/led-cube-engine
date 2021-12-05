#include <cube/core/engine.hpp>
#include <cube/hal/mock/display.hpp>
#include <cube/gfx/animation_track.hpp>
#include <cube/gfx/animations/fill_cube.hpp>
#include <chrono>

using namespace cube::core;
using namespace std::chrono;
namespace animations = cube::gfx::animations;
namespace mock = cube::hal::mock;

int main(int argc, char *argv[])
{
    engine cube_engine(new mock::display);
    animations::fill_cube fill_cube{};

    fill_cube.write_properties({
        { animations::fill_cube::cycle_interval_sec,    5s      },
        //{ animations::fill_cube::disable_red,           true    },
    });

    cube_engine.load(&fill_cube);
    cube_engine.run(); // Todo: eventually we need to cycle through animations

    return EXIT_SUCCESS;
}
