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
    bus_monitor_(context, [&](auto, auto) { ping_slaves(); }, true),
    resources_(context),
    ready_to_send_(true),
    bus_comm_(resources_.bus_comm_device)
{
    bus_monitor_.start(bus_monitor_interval);
}

void display::show(graphics_buffer const & buffer)
{
    if (!ready_to_send_)
        return; // Todo: log?

    // Update all slaves with new pixel data
    auto slave_select = resources_.pixel_comm_ss.begin();
    auto slave_addr = resources_.bus_comm_slave_addresses.begin();
    for (int i = 0; i < cube_size; ++i) {
        assert(slave_select != resources_.pixel_comm_ss.end());
        assert(slave_addr != resources_.bus_comm_slave_addresses.end());

        if (detected_slaves_.count(*slave_addr)) { // Only write to slave we actually detected
            slave_select->write(gpio::lo);
            // Todo: write pixel data
            slave_select->write(gpio::hi);
        }

        slave_select++;
        slave_addr++;
    }

    bus_comm_.broadcast<bus_command::exe_dma_swap_buffers>({}, bool_latch{ready_to_send_});
}

void display::ping_slaves()
{
    send_for_each<bus_command::exe_ping>({}, [this](auto slave, auto response) {
        if (!response) {
            detected_slaves_.erase(slave.address);

            std::cerr << "Failed to ping slave at address: " << std::to_string(slave.address) << '\n';
            return;
        }

        // Already detected the slave
        if (detected_slaves_.count(slave.address))
            return;

        // Slave detected
        // Todo: for now the order doesn't matter, but eventually it would be nice to make some helper
        // class by which we can wait for multiple commands to be handled, and only call a single
        // handler with the aggregated responses of each command.
        bus_comm_.send<bus_command::get_sys_version>({}, slave, [slave](auto response) {
            if (!response) {
                std::cerr << "Failed to get version for slave: " << std::to_string(slave.address) << '\n';
                return;
            }

            std::cout << "Found slave at address '" << std::to_string(slave.address) << "' running software version: "
                << "v" << std::to_string(response->major)
                << "." << std::to_string(response->minor)
                << "." << std::to_string(response->patch)
                << '\n';
        });

        bus_comm_.send<bus_command::exe_dma_reset>({}, slave, [this, slave](auto response) {
            if (response)
                detected_slaves_.insert(slave.address);
        });
    });
}

} // End of namespace
