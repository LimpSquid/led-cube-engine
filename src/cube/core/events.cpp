#include <cube/core/events.hpp>

using namespace std::chrono;
using std::operator""s;

namespace
{

constexpr int event_size_hint{32};

} // End of namespace

namespace cube::core
{

event_poller::event_poller() :
    fd_(::epoll_create(event_size_hint))
{
    using std::operator""s;

    if (fd_ < 0)
        throw std::runtime_error("Unable to create event_poller"s + std::strerror(errno));
}

event_poller::~event_poller()
{
    ::close(fd_);
}

bool event_poller::has_subscribers() const
{
    return !events_.empty();
}

void event_poller::subscribe(int fd, events_t events, event_handler_t * handler)
{
    using std::operator""s;

    epoll_event ev;
    ev.events = events;
    ev.data.ptr = handler;

    int r = ::epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
    if (r < 0)
        throw std::runtime_error("Failed epoll_ctl add: "s + std::strerror(errno));
    events_.push_back({});
}

void event_poller::unsubscribe(int fd)
{

    epoll_event ev{};

    int r = ::epoll_ctl(fd_, EPOLL_CTL_DEL, fd, &ev); // older kernels require a non nullptr parameter
    if (r < 0)
        throw std::runtime_error("Failed epoll_ctl delete: "s + std::strerror(errno));
    events_.pop_back();
}

void event_poller::modify(int fd, events_t events, event_handler_t * handler)
{
    using std::operator""s;

    epoll_event ev;
    ev.events = events;
    ev.data.ptr = handler;

    int r = ::epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
    if (r < 0)
        throw std::runtime_error("Failed epoll_ctl modify: "s + std::strerror(errno));
}

std::pair<int, std::reference_wrapper<std::vector<epoll_event> const>> event_poller::poll_events(std::optional<milliseconds> timeout)
{
    using std::operator""s;

    int r = ::epoll_wait(fd_, events_.data(), static_cast<int>(events_.size()), timeout ? static_cast<int>(timeout->count()) : -1);
    if (r < 0)
        throw std::runtime_error("Failed epoll_wait: "s + std::strerror(errno));
    return {r, std::cref(events_)};
}

invoker::~invoker()
{
    event_poller_.unsubscribe(fds_[1]);
    ::close(fds_[0]);
    ::close(fds_[1]);
}

void invoker::schedule()
{
     // Pipe is always writeable so the handler gets serviced as soon as the event_poller is being polled
    event_poller_.modify(fds_[1], EPOLLOUT, &handler_);
}

fd_event_notifier::fd_event_notifier(event_poller & event_poller, int fd, event_flag evs) :
    event_poller_(event_poller),
    fd_(fd)
{
    event_poller_.subscribe(fd_, evs);
}

fd_event_notifier::fd_event_notifier(event_poller & event_poller, int fd, event_handler_t handler, event_flag evs) :
    event_poller_(event_poller),
    event_handler_(std::move(handler)),
    fd_(fd)
{
    event_poller_.subscribe(fd_, evs, &event_handler_.value());
}

fd_event_notifier::~fd_event_notifier()
{
    event_poller_.unsubscribe(fd_);
}

void fd_event_notifier::set_events(event_flag evs)
{
    event_poller_.modify(fd_, evs, event_handler_ ? &event_handler_.value() : nullptr);
}

} // End of namespace
