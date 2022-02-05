#pragma once

#include <hal/rpi/gpio.hpp>
#include <hal/rpi/specs_fwd.hpp>
#include <cube/core/engine_context.hpp>
#include <array>

namespace hal::rpi
{

struct peripherals
{
    peripherals(cube::core::engine_context & /* context */)
    { }

    std::array<gpio, cube_size> spi_ss_gpios
    {
        make_output(05), make_output(22), make_output(17), make_output(04),
        make_output(18), make_output(23), make_output(24), make_output(25),
        make_output(12), make_output(16), make_output(20), make_output(21),
        make_output(26), make_output(19), make_output(13), make_output(06),
    };
};

} // End of namespace
