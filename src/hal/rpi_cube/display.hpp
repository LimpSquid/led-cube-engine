#pragma once

#include <hal/rpi_cube/resources.hpp>
#include <hal/rpi_cube/bus_comm.hpp>
#include <cube/core/graphics_device.hpp>
#include <cube/core/timers.hpp>
#include <unordered_set>

namespace hal::rpi_cube
{

class display;
namespace detail
{

struct async_pixel_pump;

class display_shutdown_signal :
    public cube::core::engine_shutdown_signal
{
public:
    display_shutdown_signal(display & display);

private:
    void shutdown_requested() override;
    display & display_;
};

} // End of namespace

class display :
    public cube::core::graphics_device
{
public:
    display(cube::core::engine_context & context);
    ~display();

private:
    friend class detail::display_shutdown_signal;
    friend struct detail::async_pixel_pump;

    int map_to_offset(int x, int y, int z) const override;
    void show(cube::core::graphics_buffer const & buffer) override;

    template<bus_command C, typename H>
    void send_for_each(bus_request_params<C> params, H handler)
    {
        for (auto address : resources_.bus_comm_slave_addresses)
            bus_comm_.send<C>(params, bus_node{address}, [address, handler](auto && response) {
                handler(bus_node{address}, std::move(response));
            });
    }

    template<bus_command C, typename H>
    void send_for_all(bus_request_params<C> params, H handler)
    {
        struct session
        {
            std::vector<std::pair<bus_node, bus_response_params_or_error<C>>> responses;
        };

        auto s = std::make_shared<session>();

        for (auto address : resources_.bus_comm_slave_addresses)
            bus_comm_.send<C>(params, bus_node{address}, [address, handler, s](auto && response) {
                s->responses.push_back(std::make_pair(bus_node{address}, std::move(response)));
                if (s.use_count() == 1)
                    handler(std::move(s->responses));
            });
    }

    void pixel_pump_finished();
    void probe_slaves();

    cube::core::recurring_timer bus_monitor_;
    std::unordered_set<unsigned char> detected_slaves_;
    resources resources_;

    // Keep last
    bus_comm bus_comm_;
    detail::display_shutdown_signal shutdown_signal_;
    std::unique_ptr<detail::async_pixel_pump const> pixel_pump_;
};

} // End of namespace

namespace hal { using graphics_device_t = rpi_cube::display; }
