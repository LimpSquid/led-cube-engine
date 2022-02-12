#include <hal/rpi_cube/display.hpp>

using namespace cube::core;

namespace hal::rpi_cube
{

display::display(engine_context & context) :
    graphics_device(context),
    resources_(context)
{

}

void display::show(graphics_buffer const & buffer)
{
    // Send graphics buffer to slaves, swap frame buffers etc.
}

} // End of namespace
