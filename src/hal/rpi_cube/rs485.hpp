#pragma once

#include <hal/rpi_cube/gpio.hpp>
#include <termios.h>

namespace cube::core { class engine_context; }
namespace hal::rpi_cube
{

class gpio;
struct rs485_config
{
    char const * device;
    speed_t baudrate;
    unsigned int dir_pin;
};

class rs485
{
public:
    rs485(rs485_config config, cube::core::engine_context & io_context);

private:
    rs485(rs485 & other) = delete;
    rs485(rs485 && other) = delete;

    gpio const dir_gpio;
};

} // End of namespace
