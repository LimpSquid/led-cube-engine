#pragma once

#include <hal/rpi_cube/resources.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <cube/core/graphics_device.hpp>
#include <cube/core/timers.hpp>
#include <unordered_set>

namespace hal::rpi_cube
{

class display :
    public cube::core::graphics_device
{
public:
    display(cube::core::engine_context & context);

private:
    virtual void show(cube::core::graphics_buffer const & buffer) override;

    template<bus_command C, typename H>
    void send_for_each(bus_request_params<C> params, H handler)
    {
        for (auto address : resources_.bus_comm_slave_addresses)
            bus_comm_.send<C>(params, bus_comm::node{address}, [address, h = std::move(handler)](auto && response) {
                h(bus_comm::node{address}, std::move(response));
            });
    }

    void ping_slaves();

    cube::core::recurring_timer bus_monitor_;
    std::unordered_set<unsigned char> detected_slaves_;
    resources resources_;
    bool ready_to_send_;

    // Keep last
    bus_comm bus_comm_;
};

} // End of namespace

namespace hal { using graphics_device_t = rpi_cube::display; }
