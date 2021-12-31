#include <hal/rpi/voxel_display.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace cube::core;

namespace hal::rpi
{

voxel_display::voxel_display(engine_context & context) :
    graphics_device(context)
{

}

void voxel_display::show(graphics_buffer const & buffer)
{
    // Send graphics buffer to slaves, swap frame buffers etc.
}

} // End of namespace
