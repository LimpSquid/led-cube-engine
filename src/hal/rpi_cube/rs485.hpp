#pragma once

#include <hal/rpi_cube/gpio.hpp>
#include <cube/core/events.hpp>
#include <boost/circular_buffer.hpp>
#include <termios.h>
#include <mutex>
#include <thread>

namespace cube::core { class engine_context; }
namespace hal::rpi_cube
{

struct rs485_config
{
    char const * const device;
    speed_t baudrate;
    unsigned int dir_pin;
};

class rs485
{
public:
    enum channel
    {
        rx,
        tx
    };

    rs485(rs485_config config, cube::core::engine_context & context);
    ~rs485();

    template<typename T>
    bool is_readable() const
    {
        // Makes no sense to check if you can read a certain type, without actually
        // going to read it, so do the same check here.
        static_assert(std::is_trivially_copyable_v<T>);
        return bytes_available(rx) >= sizeof(T);
    }

    template<typename T>
    bool is_writeable() const
    {
        // Makes no sense to check if you can read a certain type, without actually
        // going to read it, so do the same check here.
        static_assert(std::is_trivially_copyable_v<T>);
        return bytes_available(tx) >= sizeof(T);
    }

    template<typename T>
    void read_into(T & dst)
    {
        static_assert(std::is_trivially_copyable_v<T>); // Because we're memcpying buffer into out
        std::size_t size = read(&dst, sizeof(T));
        if (size != sizeof(T))
            throw std::runtime_error("Read only " + std::to_string(size) + " bytes, expected " + std::to_string(sizeof(T)));
    }

    template<typename T>
    void write_from(T const & src)
    {
        static_assert(std::is_trivially_copyable_v<T>); // Because we're memcpying in into buffer
        std::size_t size = write(&src, sizeof(T));
        if (size != sizeof(T))
            throw std::runtime_error("Written only " + std::to_string(size) + " bytes, expected " + std::to_string(sizeof(T)));
    }

    std::size_t bytes_available(channel ch) const;
    std::size_t read(void * dst, std::size_t count);
    std::size_t write(void const * src, std::size_t count);

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
    char chunk_buffer_[512]; // Max size of single read/write
    gpio const dir_gpio_;
    std::mutex lock_;
    std::thread drain_thread_;
    bool draining_;
};

} // End of namespace
