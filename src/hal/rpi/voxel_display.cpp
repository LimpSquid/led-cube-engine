#include <hal/rpi/voxel_display.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace cube::core;

namespace hal::rpi
{

voxel_display::voxel_display()
{

}

voxel_display::~voxel_display()
{

}

void voxel_display::show(graphics_buffer const & buffer)
{
    // Send graphics buffer to slaves, swap frame buffers etc.
}

void voxel_display::poll()
{
    // Do other hardware stuff, like bus communication?
}

} // End of namespace
