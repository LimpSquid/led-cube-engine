#pragma once

#include <hal/rpi_cube/iodev.hpp>
#include <hal/rpi_cube/gpio.hpp>
#include <cube/core/events.hpp>
#include <boost/circular_buffer.hpp>
#include <termios.h>
#include <mutex>
#include <thread>

namespace hal::rpi_cube
{

struct rs485_config
{
    char const * const device;
    speed_t baudrate;
    unsigned int dir_pin;
};

class rs485
    : public iodev
{
public:
    rs485(rs485_config config, cube::core::engine_context & context);
    ~rs485();

private:
    rs485(rs485 &) = delete;
    rs485(rs485 &&) = delete;

    std::size_t bytes_avail_for_reading() const override;
    std::size_t bytes_avail_for_writing() const override;
    std::size_t read(void * dst, std::size_t count) override;
    std::size_t write(void const * src, std::size_t count) override;
    void clear_input() override;
    void clear_output() override;

    void on_event(cube::core::fd_event_notifier::event_flags evs);
    void throw_error();
    void read_into_buffer();
    void write_from_buffer();

    cube::core::safe_fd fd_;
    cube::core::fd_event_notifier event_notifier_;
    boost::circular_buffer<char> tx_buffer_;
    boost::circular_buffer<char> rx_buffer_;
    char chunk_buffer_[64]; // Max size of single read/write, don't make this too large otherwise the direction pin gets pulled low too early
    gpio const dir_gpio_;
    std::mutex lock_;
    std::thread drain_thread_;
    bool draining_;
};

} // End of namespace
