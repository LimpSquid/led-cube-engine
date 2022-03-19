#include <hal/rpi_cube/display.hpp>
#include <iostream>
#include <chrono>

using namespace std::chrono;

using namespace cube::core;

namespace
{

constexpr seconds bus_monitor_interval{5};

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
    bus_monitor_(context, [&](auto, auto) { ping_nodes(); }),
    resources_(context),
    bus_comm_(resources_.bus_comm_device),
    ready_to_send_(true)
{
    send_for_each<bus_command::get_sys_version>({}, [](auto node, auto response) {
        if (!response)
            std::cerr << "Failed to get version for target: " << std::to_string(node.address) << '\n';
        std::cout << "Found target: " << std::to_string(node.address) << " with version: "
            << "v" << std::to_string(response->major)
            << "." << std::to_string(response->minor)
            << "." << std::to_string(response->patch)
            << '\n';
    });

    bus_monitor_.start(bus_monitor_interval);
}

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

void display::ping_nodes()
{
    send_for_each<bus_command::exe_ping>({}, [](auto node, auto response) {
        if (!response)
            std::cerr << "Failed to ping target: " << std::to_string(node.address) << '\n';
    });
}

} // End of namespace
