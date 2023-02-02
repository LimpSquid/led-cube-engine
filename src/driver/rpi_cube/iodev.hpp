#pragma once

#include <cube/core/timers.hpp>
#include <stdexcept>
#include <functional>
#include <unordered_map>

namespace driver::rpi_cube
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

class iodev_metrics_logger
{
public:
    iodev_metrics_logger(cube::core::engine_context & context, char const * const name);

    void bytes_written(std::size_t bytes) { bytes_written_ += bytes; }
    void bytes_read(std::size_t bytes) { bytes_read_ += bytes; }

private:
    struct snapshot_last_log
    {
        std::size_t bytes_written{0};
        std::size_t bytes_read{0};
        std::chrono::seconds epoch{std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch())};
    };

    iodev_metrics_logger(iodev_metrics_logger const &) = delete;
    iodev_metrics_logger(iodev_metrics_logger &&) = delete;

    cube::core::engine_context & context_;
    cube::core::recurring_timer timer_;

    snapshot_last_log snapshot_;
    std::size_t bytes_written_;
    std::size_t bytes_read_;
};

class iodev
{
public:
    using subscription_handler_t = std::function<void()>;

    enum direction
    {
        input,
        output,
        all_directions
    };

    enum subscription
    {
        ready_read,
        transfer_complete
    };

    cube::core::engine_context & context();
    void clear(direction dir);
    iodev_subscription subscribe(subscription type, subscription_handler_t handler);

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
        metrics_logger_.bytes_read(size);
        if (size != sizeof(T))
            throw std::runtime_error("Read only " + std::to_string(size) + " bytes, expected " + std::to_string(sizeof(T)));
    }

    template<typename T>
    void write_from(T const & src)
    {
        static_assert(std::is_trivially_copyable_v<T>); // Because we're memcpying src into buffer
        std::size_t size = write(&src, sizeof(T));
        metrics_logger_.bytes_written(size);
        if (size != sizeof(T))
            throw std::runtime_error("Written only " + std::to_string(size) + " bytes, expected " + std::to_string(sizeof(T)));
    }

protected:
    iodev(cube::core::engine_context & context, char const * const name);

    // For now this **MUST** always be called, eventually we could fallback on polling
    void notify_readable() const;
    void notify_transfer_complete() const;

private:
    friend class iodev_subscription;

    struct subscription_info
    {
        subscription type;
        subscription_handler_t handler;
    };

    virtual std::size_t bytes_avail_for_reading() const = 0;
    virtual std::size_t bytes_avail_for_writing() const = 0;
    virtual std::size_t read(void * dst, std::size_t count) = 0;
    virtual std::size_t write(void const * src, std::size_t count) = 0;
    virtual void clear_input() = 0;
    virtual void clear_output() = 0;

    int next_subscription();
    void unsubscribe(int id);

    cube::core::engine_context & context_;
    std::unordered_map<int, subscription_info> subscriptions_;
    iodev_metrics_logger metrics_logger_;
    int subscription_id_;
};

} // End of namespace
