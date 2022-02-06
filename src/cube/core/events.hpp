#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <optional>
#include <cstring>
#include <chrono>

// @Todo: maybe move some of this to a source file

namespace cube::core
{

using events_t = uint32_t;
using event_handler_t = std::function<void(events_t)>;

class event_poller
{
public:
    event_poller();
    ~event_poller();

    bool has_subscribers() const;
    void subscribe(int fd, events_t events = 0, event_handler_t * handler = nullptr);
    void unsubscribe(int fd);
    void modify(int fd, events_t events = 0, event_handler_t * handler = nullptr);

    std::pair<int, std::reference_wrapper<std::vector<epoll_event> const>> poll_events(std::optional<std::chrono::milliseconds> timeout = {});

private:
    event_poller(event_poller & other) = delete;
    event_poller(event_poller && other) = delete;

    std::vector<epoll_event> events_;
    int fd_;
};

class invoker
{
public:
    template<typename T>
    invoker(event_poller & event_poller, T handler) :
        event_poller_(event_poller),
        handler_([this, h = std::move(handler)](events_t) {
            event_poller_.modify(fds_[1]);
            h();
        })
    {
        using std::operator""s;

        int r = ::pipe(fds_);
        if (r < 0)
            throw std::runtime_error("Failed to create pipe: "s + std::strerror(errno));
        event_poller_.subscribe(fds_[1]);
    }

    ~invoker();

    void schedule();

private:
    invoker(invoker & other) = delete;
    invoker(invoker && other) = delete;

    event_poller & event_poller_;
    event_handler_t handler_;
    int fds_[2];
};

class fd_event_notifier
{
public:
    enum event_flags : events_t
    {
        none    = 0,
        read    = EPOLLIN,
        write   = EPOLLOUT,
        err     = EPOLLERR,
    };

    friend event_flags operator|(event_flags lhs, event_flags rhs) { return static_cast<event_flags>(lhs | rhs); }
    friend event_flags operator&(event_flags lhs, event_flags rhs) { return static_cast<event_flags>(lhs & rhs); }

    using handler_t = std::function<void(event_flags)>;

    fd_event_notifier(event_poller & event_poller, int fd, event_flags evs = none);
    fd_event_notifier(event_poller & event_poller, int fd, event_flags evs, handler_t handler);
    ~fd_event_notifier();

    void set_events(event_flags evs);

private:
    struct handler_relay
    {
        handler_relay(handler_t h) :
            handler(std::move(h))
        { }

        void operator()(events_t evs)
        {
            handler(static_cast<event_flags>(evs));
        }

        handler_t handler;
    };

    fd_event_notifier(fd_event_notifier & other) = delete;
    fd_event_notifier(fd_event_notifier && other) = delete;

    event_poller & event_poller_;
    std::optional<event_handler_t> event_handler_;
    int fd_;
};

} // End of namespace
