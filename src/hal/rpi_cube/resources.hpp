#pragma once

#include <hal/rpi_cube/gpio.hpp>
#include <hal/rpi_cube/rs485.hpp>
#include <hal/rpi_cube/specs_fwd.hpp>
#include <array>

namespace hal::rpi_cube
{

constexpr rs485_config bus_comm_config
{
    "/dev/serial0", // device
    B115200,        // baudrate
    2,              // gpio dir pin
};

// Wrap all resources in order to provide a defined state on acquisition (and release).
struct resources
{
    resources(cube::core::engine_context & context) :
        bus_comm_device(bus_comm_config, context)
    {
        for (gpio const & ss : pixel_comm_ss)
            ss.write(gpio::hi);
    }

    ~resources()
    {
        for (gpio const & ss : pixel_comm_ss)
            ss.write(gpio::hi);
    }

    rs485 bus_comm_device;
    std::array<unsigned char, cube_size> const bus_comm_addresses
    {
         0,  1,  2,  3,
         4,  5,  6,  7,
         8,  9, 10, 11,
        12, 13, 14, 15,
    };

    std::array<gpio, cube_size> const pixel_comm_ss
    {
        make_output( 5), make_output(22), make_output(17), make_output( 4),
        make_output(18), make_output(23), make_output(24), make_output(25),
        make_output(12), make_output(16), make_output(20), make_output(21),
        make_output(26), make_output(19), make_output(13), make_output(06),
    };
};

} // End of namespace
