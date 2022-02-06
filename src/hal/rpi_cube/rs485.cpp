#include <hal/rpi_cube/rs485.hpp>

namespace hal::rpi_cube
{

rs485::rs485(rs485_config config, cube::core::engine_context & /* io_context */) :
    dir_gpio(config.dir_pin, gpio::output)
{

}

} // End of namespace
