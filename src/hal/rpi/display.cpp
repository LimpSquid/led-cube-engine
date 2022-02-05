#include <hal/rpi/display.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace cube::core;

namespace hal::rpi
{

display::display(engine_context & context) :
    graphics_device(context),
    peripherals_(context)
{

}

void display::show(graphics_buffer const & buffer)
{
    // Send graphics buffer to slaves, swap frame buffers etc.
}

} // End of namespace
