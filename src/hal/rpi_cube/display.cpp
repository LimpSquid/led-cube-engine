#include <hal/rpi_cube/display.hpp>

using namespace cube::core;

namespace hal::rpi_cube
{

display::display(engine_context & context) :
    graphics_device(context),
    resources_(context),
    bus_comm_(resources_.bus_comm_device)
{

}

void display::show(graphics_buffer const & buffer)
{

}

} // End of namespace
