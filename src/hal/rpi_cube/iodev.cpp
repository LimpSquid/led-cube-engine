#include <hal/rpi_cube/iodev.hpp>

using namespace cube::core;

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

iodev_subscription iodev::subscribe(iodev_read_handler_t handler)
{
    int id = next_subscription();
    read_handlers_[id] = std::move(handler);
    return {*this, id};
}

engine_context & iodev::context()
{
    return context_;
}

iodev::iodev(engine_context & context) :
    context_(context),
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
