#pragma once

#include <driver/rpi_cube/resources.hpp>
#include <driver/rpi_cube/bus_comm.hpp>
#include <cube/core/graphics_device.hpp>
#include <cube/core/timers.hpp>
#include <unordered_set>

namespace driver::rpi_cube
{

class display;

namespace detail
{

struct rgb_buffer;
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

    void pixel_pump_run();
    void pixel_pump_finished();
    void probe_slaves();

    cube::core::recurring_timer bus_monitor_;
    std::unordered_set<unsigned char> detected_slaves_;
    resources resources_;

    // Keep last
    bus_comm bus_comm_;
    detail::display_shutdown_signal shutdown_signal_;
    std::deque<std::unique_ptr<detail::rgb_buffer const>> buffer_queue_;
    std::unique_ptr<detail::async_pixel_pump const> pixel_pump_;
};

} // End of namespace
