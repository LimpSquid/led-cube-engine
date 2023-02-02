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
        throw_errno("create event_poller");
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

    // In case the fd is already closed but we haven't unsubscribed it yet
    if (has_subscription(fd))
        throw std::runtime_error("Already subscribed to fd: " + std::to_string(fd));

    int r = ::epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
    if (r < 0)
        throw_errno("epoll_ctl add");
    events_.push_back({});
    fds_.insert(fd);
}

void event_poller::unsubscribe(int fd)
{
    epoll_event ev{};

    int r = ::epoll_ctl(fd_, EPOLL_CTL_DEL, fd, &ev); // older kernels require a non nullptr parameter

    // It might be possible that the fd is already closed but we still hold a reference to it.
    // For this particular case we allow the EBADF (invalid file descriptor) error.
    if (r < 0 && !(has_subscription(fd) && errno == EBADF))
        throw_errno("epoll_ctl delete");
    events_.pop_back();
    fds_.erase(fd);
}

void event_poller::modify(int fd, events_t events, event_handler_t * handler)
{
    using std::operator""s;

    epoll_event ev;
    ev.events = events;
    ev.data.ptr = handler;

    int r = ::epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
    if (r < 0)
        throw_errno("epoll_ctl modify");
}

std::pair<int, std::reference_wrapper<std::vector<epoll_event> const>> event_poller::poll_events(std::optional<milliseconds> timeout)
{
    using std::operator""s;

    int r = ::epoll_wait(fd_, events_.data(), static_cast<int>(events_.size()), timeout ? static_cast<int>(timeout->count()) : -1);
    if (r < 0) {
        if (errno == EINTR)
            return {0, std::cref(events_)};
        throw_errno("epoll_wait");
    }

    return {r, std::cref(events_)};
}

function_invoker::~function_invoker()
{
    event_poller_.unsubscribe(fds_[1]);
}

void function_invoker::schedule()
{
     // Pipe is always writeable so the handler gets serviced as soon as the event_poller is being polled
    event_poller_.modify(fds_[1], EPOLLOUT, &handler_);
}

fd_event_notifier::fd_event_notifier(event_poller & event_poller, int fd, event_flags evs) :
    event_poller_(event_poller),
    evs_(evs),
    fd_(fd)
{
    event_poller_.subscribe(fd_, evs_);
}

fd_event_notifier::fd_event_notifier(event_poller & event_poller, int fd, event_flags evs, handler_t handler) :
    event_poller_(event_poller),
    event_handler_([h = std::move(handler)](events_t evs){ h(static_cast<event_flags>(evs)); }),
    evs_(evs),
    fd_(fd)
{
    event_poller_.subscribe(fd_, evs_, &event_handler_.value());
}

fd_event_notifier::~fd_event_notifier()
{
    event_poller_.unsubscribe(fd_);
}

void fd_event_notifier::set_events(event_flags evs)
{
    evs_ |= evs;
    event_poller_.modify(fd_, evs_, event_handler_ ? &event_handler_.value() : nullptr);
}

void fd_event_notifier::clr_events(event_flags evs)
{
    evs_ &= ~evs;
    event_poller_.modify(fd_, evs_, event_handler_ ? &event_handler_.value() : nullptr);
}

} // End of namespace
