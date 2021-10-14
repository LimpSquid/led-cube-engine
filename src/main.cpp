#include <cube/core/engine.h>
#include <cube/hal/voxel_display.h>
#include <cube/gfx/animation_player.h>
#include <cube/gfx/animation_track.h>
#include <cube/gfx/animations/fill_cube.h>

int main(int argc, char *argv[])
{
    cube::core::engine cube_engine(new cube::hal::voxel_display);
    cube::gfx::animation_player<cube::gfx::property_animation_track> player;

    return EXIT_SUCCESS;
}
