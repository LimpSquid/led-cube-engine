#include <hal/rpi_cube/display.hpp>
#include <cube/core/composite_function.hpp>
#include <iostream>
#include <chrono>

using namespace cube::core;
using namespace std::chrono;

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

struct rgb_buffer
{
    struct red_tag   { using offset = std::integral_constant<std::size_t, 0 * cube::cube_size_2d>; };
    struct green_tag { using offset = std::integral_constant<std::size_t, 1 * cube::cube_size_2d>; };
    struct blue_tag  { using offset = std::integral_constant<std::size_t, 2 * cube::cube_size_2d>; };

    struct slice_buffer
    {
        template<typename Tag>
        auto begin() { return data.begin() + Tag::offset::value; }
        template<typename Tag>
        auto const begin() const { return data.begin() + Tag::offset::value; }

        template<typename Tag>
        auto end() { return begin<Tag>() + cube::cube_size_1d; }
        template<typename Tag>
        auto const end() const { return begin<Tag>() + cube::cube_size_1d; }

        std::array<color_t, 3 * cube::cube_size_2d> data;
    };

    std::array<slice_buffer, cube::cube_size_1d> slices;
};
static_assert(sizeof(rgb_buffer) == (3 * sizeof(uint8_t) * cube::cube_size_3d));
static_assert(std::is_trivially_copyable_v<rgb_buffer>);

rgb_buffer transform(graphics_buffer const & buffer)
{
    rgb_buffer rgbb;
    auto rgba = buffer.begin();

    for (auto & slice : rgbb.slices) {
        auto r = slice.begin<rgb_buffer::red_tag>();
        auto g = slice.begin<rgb_buffer::green_tag>();
        auto b = slice.begin<rgb_buffer::blue_tag>();

        while (r != slice.end<rgb_buffer::red_tag>()) {
            *r++ = static_cast<color_t>(*rgba   >> 24);
            *g++ = static_cast<color_t>(*rgba   >> 16);
            *b++ = static_cast<color_t>(*rgba++ >>  8);
        }
    }

    return rgbb;
}

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

int display::map_to_offset(int x, int y, int z) const
{
    return x + z * cube::cube_size_1d + y * cube::cube_size_2d;
}

void display::show(graphics_buffer const & buffer)
{
    if (!ready_to_send_)
        return; // Todo: log

    rgb_buffer const rgbb = transform(buffer); // Ideally we avoid this transform altogether

    // Todo: Maybe we want to schedule this in the event loop and chop it up
    // into "cube_size_1d" number of function calls as writing to the pixel_comm_device
    // is a blocking operation.

    // Update all slaves with new pixel data
    auto slave_select = resources_.pixel_comm_ss.begin();
    auto slave_addr = resources_.bus_comm_slave_addresses.begin();
    for (int i = 0; i < cube::cube_size_1d; ++i) {
        assert(slave_select != resources_.pixel_comm_ss.end());
        assert(slave_addr != resources_.bus_comm_slave_addresses.end());

        if (detected_slaves_.count(*slave_addr)) { // Only write to the slaves we actually detected
            gpio_lo_guard gpio_guard{*slave_select};
            resources_.pixel_comm_device.write_from(rgbb.slices[i]);
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

        // New slave detected, initialize
        auto [sys_version_handler, dma_reset_handler] = decompose([this, slave](
            bus_response_params_or_error<bus_command::get_sys_version> version_response,
            bus_response_params_or_error<bus_command::exe_dma_reset> reset_response) {
                if (!version_response || ! reset_response) {
                    std::cerr << "Failed to initialize slave: " << std::to_string(slave.address) << '\n';
                    return;
                }

                std::cout << "Found slave at address '" << std::to_string(slave.address) << "' running software version: "
                    << "v" << std::to_string(version_response->major)
                    << "." << std::to_string(version_response->minor)
                    << "." << std::to_string(version_response->patch)
                    << '\n';
                detected_slaves_.insert(slave.address);
            }
        );

        bus_comm_.send<bus_command::get_sys_version>({}, slave, sys_version_handler);
        bus_comm_.send<bus_command::exe_dma_reset>({}, slave, dma_reset_handler);
    });
}

} // End of namespace
