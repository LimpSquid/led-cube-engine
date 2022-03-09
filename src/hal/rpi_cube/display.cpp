#include <hal/rpi_cube/display.hpp>
#include <iostream> // Todo: remove

using namespace cube::core;

namespace
{

struct bool_latch
{
    bool_latch(bool & b) :
        b_(b)
    {
        b_ = false;
    }

    void operator()()
    {
        b_ = true;
    }

private:
    bool & b_;
};

} // End of namespace

namespace hal::rpi_cube
{

display::display(engine_context & context) :
    graphics_device(context),
    resources_(context),
    bus_comm_(resources_.bus_comm_device),
    ready_to_send_(true)
{ }

void display::show(graphics_buffer const & buffer)
{
    if (!ready_to_send_)
        return; // Todo: log?

    // Update all slaves with new pixel data
    auto slave_select = resources_.pixel_comm_ss.begin();
    for (int i = 0; i < cube_size; ++i) {
        slave_select->write(gpio::lo);
        // Todo: write pixel data
        slave_select->write(gpio::hi);
        slave_select++;
    }

    bus_comm_.broadcast<bus_command::exe_dma_swap_buffers>({}, bool_latch{ready_to_send_});
}

} // End of namespace
