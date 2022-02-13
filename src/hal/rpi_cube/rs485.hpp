#pragma once

#include <hal/rpi_cube/gpio.hpp>
#include <cube/core/events.hpp>
#include <boost/circular_buffer.hpp>
#include <termios.h>

namespace cube::core { class engine_context; }
namespace hal::rpi_cube
{

class gpio;
struct rs485_config
{
    char const * const device;
    speed_t baudrate;
    unsigned int dir_pin;
};

class rs485
{
public:
    rs485(rs485_config config, cube::core::engine_context & context);
    ~rs485();

private:
    rs485(rs485 & other) = delete;
    rs485(rs485 && other) = delete;

    void on_event(cube::core::fd_event_notifier::event_flags evs);
    void throw_error();
    void read_into_buffer();
    void write_from_buffer();

    cube::core::safe_fd fd_;
    cube::core::fd_event_notifier event_notifier_;
    boost::circular_buffer<char> tx_buffer_;
    boost::circular_buffer<char> rx_buffer_;
    char chunk_buffer_[256];
    gpio const dir_gpio_;
};

} // End of namespace
