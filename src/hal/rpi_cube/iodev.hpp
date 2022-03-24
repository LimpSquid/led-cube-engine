#pragma once

#include <stdexcept>
#include <functional>
#include <unordered_map>

namespace cube::core { class engine_context; }
namespace hal::rpi_cube
{

class iodev;
class iodev_subscription
{
public:
    iodev_subscription(iodev_subscription && other);
    ~iodev_subscription();

private:
    friend class iodev;

    iodev_subscription(iodev & device, int id);

    void unsubscribe();

    iodev & device_;
    int id_;
};

using iodev_read_handler_t = std::function<void()>;
class iodev
{
public:
    enum direction
    {
        input,
        output,
        all_directions
    };

    cube::core::engine_context & context();
    void clear(direction dir);
    iodev_subscription subscribe(iodev_read_handler_t handler);

    template<typename T>
    bool is_readable() const
    {
        // Makes no sense to check if you can read a certain type, without actually
        // going to read it, so do the same check here.
        static_assert(std::is_trivially_copyable_v<T>);
        return bytes_avail_for_reading() >= sizeof(T);
    }

    template<typename T>
    bool is_writeable() const
    {
        // Makes no sense to check if you can read a certain type, without actually
        // going to write it, so do the same check here.
        static_assert(std::is_trivially_copyable_v<T>);
        return bytes_avail_for_writing() >= sizeof(T);
    }

    template<typename T>
    void read_into(T & dst)
    {
        static_assert(std::is_trivially_copyable_v<T>); // Because we're memcpying buffer into dst
        std::size_t size = read(&dst, sizeof(T));
        if (size != sizeof(T))
            throw std::runtime_error("Read only " + std::to_string(size) + " bytes, expected " + std::to_string(sizeof(T)));
    }

    template<typename T>
    void write_from(T const & src)
    {
        static_assert(std::is_trivially_copyable_v<T>); // Because we're memcpying src into buffer
        std::size_t size = write(&src, sizeof(T));
        if (size != sizeof(T))
            throw std::runtime_error("Written only " + std::to_string(size) + " bytes, expected " + std::to_string(sizeof(T)));
    }

protected:
    iodev(cube::core::engine_context & context);

    void notify_readable() const; // For now this **MUST** always be called, eventually we could fallback on polling

private:
    friend class iodev_subscription;

    virtual std::size_t bytes_avail_for_reading() const = 0;
    virtual std::size_t bytes_avail_for_writing() const = 0;
    virtual std::size_t read(void * dst, std::size_t count) = 0;
    virtual std::size_t write(void const * src, std::size_t count) = 0;
    virtual void clear_input() = 0;
    virtual void clear_output() = 0;

    int next_subscription();
    void unsubscribe(int id);

    cube::core::engine_context & context_;
    std::unordered_map<int, iodev_read_handler_t> read_handlers_;
    int subscription_id_;
};

} // End of namespace
