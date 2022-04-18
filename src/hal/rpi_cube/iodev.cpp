#include <hal/rpi_cube/iodev.hpp>
#include <cube/core/logging.hpp>

using namespace cube::core;
using namespace std::chrono;

namespace hal::rpi_cube
{

iodev_subscription::iodev_subscription(iodev_subscription && other) :
    device_(other.device_),
    id_(other.id_)
{
    other.id_ = -1;
}

iodev_subscription::~iodev_subscription()
{
    unsubscribe();
}

iodev_subscription::iodev_subscription(iodev & device, int id) :
    device_(device),
    id_(id)
{ }


void iodev_subscription::unsubscribe()
{
    if (id_ >= 0) {
        device_.unsubscribe(id_);
        id_ = -1;
    }
}

iodev_metrics_logger::iodev_metrics_logger(cube::core::engine_context & context, char const * const name) :
    context_(context),
    timer_(context, [this, name](auto now, auto) {
        seconds now_s = duration_cast<seconds>(now.time_since_epoch());
        seconds elapsed = now_s - snapshot_.epoch;
        long double write_rate = (bytes_written_ - snapshot_.bytes_written) / (1024.0l * elapsed.count());
        long double read_rate = (bytes_read_ - snapshot_.bytes_read) / (1024.0l * elapsed.count());

        LOG_DBG("IO device metrics",
            LOG_ARG("name", name),
            LOG_ARG("bytes_written", bytes_written_),
            LOG_ARG("bytes_read", bytes_read_),
            LOG_ARG("avg_write_rate", std::to_string(write_rate) + " KiB/s"),
            LOG_ARG("avg_read_rate", std::to_string(read_rate) + " KiB/s"));

        snapshot_.epoch = now_s;
        snapshot_.bytes_written = bytes_written_;
        snapshot_.bytes_read = bytes_read_;
    }),
    bytes_written_(0),
    bytes_read_(0)
{
    timer_.start(30s);
}

engine_context & iodev::context()
{
    return context_;
}

void iodev::clear(direction dir)
{
    if (dir == all_directions || dir == input)
        clear_input();
    if (dir == all_directions || dir == output)
        clear_output();
}

iodev_subscription iodev::subscribe(iodev_read_handler_t handler)
{
    int id = next_subscription();
    read_handlers_[id] = std::move(handler);
    return {*this, id};
}

iodev::iodev(engine_context & context, char const * const name) :
    context_(context),
    metrics_logger_(context, name),
    subscription_id_(0)
{ }

void iodev::notify_readable() const
{
    for (auto const & [_, handler] : read_handlers_)
        handler();
}

int iodev::next_subscription()
{
    return subscription_id_++;
}

void iodev::unsubscribe(int id)
{
    read_handlers_.erase(id);
}

} // End of namespace
