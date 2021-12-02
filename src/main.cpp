#include <cube/core/engine.h>
#include <cube/hal/mock/opengl_display.h>
#include <cube/gfx/animation_track.h>
#include <cube/gfx/animations/fill_cube.h>

int main(int argc, char *argv[])
{
    cube::core::engine cube_engine(new cube::hal::opengl_display);
    cube::gfx::animations::fill_cube fill_cube{};

    cube_engine.load(&fill_cube);
    cube_engine.run(); // Todo: eventually we need to cycle through animations

    return EXIT_SUCCESS;
}
