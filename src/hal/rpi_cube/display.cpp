#include <hal/rpi_cube/display.hpp>
#include <iostream> // Todo: remove

using namespace cube::core;

namespace hal::rpi_cube
{

display::display(engine_context & context) :
    graphics_device(context),
    resources_(context),
    bus_comm_(resources_.bus_comm_device)
{
    // Todo: remove
    bus_comm_.send<bus_command::get_sys_version>({}, {0x00},
        [](auto version) {
            if (!version)
                throw std::runtime_error("Unable to get version number");
            std::cout
                << "major: " << version->major
                << "minor: " << version->minor
                << "patch: " << version->patch << '\n';
        }
    );

    bus_comm_.broadcast<bus_command::exe_dma_swap_buffers>({});
}

void display::show(graphics_buffer const & buffer)
{

}

} // End of namespace
