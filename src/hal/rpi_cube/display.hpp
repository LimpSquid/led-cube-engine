#pragma once

#include <hal/rpi_cube/resources.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <cube/core/graphics_device.hpp>
#include <cube/core/timers.hpp>
#include <unordered_set>

namespace hal::rpi_cube
{

namespace detail { struct async_pixel_pump; }

class display :
    public cube::core::graphics_device,
    private cube::core::engine_shutdown_signal
{
public:
    display(cube::core::engine_context & context);
    ~display();

private:
    int map_to_offset(int x, int y, int z) const override;
    void show(cube::core::graphics_buffer const & buffer) override;
    void shutdown_requested() override;

    template<bus_command C, typename H>
    void send_for_each(bus_request_params<C> params, H handler)
    {
        for (auto address : resources_.bus_comm_slave_addresses)
            bus_comm_.send<C>(params, bus_node{address}, [address, handler](auto && response) {
                handler(bus_node{address}, std::move(response));
            });
    }

    template<bus_command C, typename H>
    void send_all(bus_request_params<C> params, H handler)
    {
        auto ref = std::make_shared<std::size_t>(resources_.bus_comm_slave_addresses.size());
        for (auto address : resources_.bus_comm_slave_addresses)
            bus_comm_.send<C>(params, bus_node{address}, [handler, ref](auto &&) {
                if (--*ref == 0)
                    handler(); // TODO: eventually pass in success/error responses?
            });
    }

    void ping_slaves();

    cube::core::recurring_timer bus_monitor_;
    std::unordered_set<unsigned char> detected_slaves_;
    resources resources_;
    bool ready_to_send_;

    // Keep last
    bus_comm bus_comm_;
    std::unique_ptr<detail::async_pixel_pump const> pixel_pump_;
};

} // End of namespace

namespace hal { using graphics_device_t = rpi_cube::display; }
