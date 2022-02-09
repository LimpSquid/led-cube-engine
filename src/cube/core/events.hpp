#pragma once

#include <cube/core/utils.hpp>
#include <sys/epoll.h>
#include <functional>
#include <optional>
#include <cstring>
#include <chrono>
#include <stdexcept>

namespace cube::core
{

using events_t = uint32_t;
using event_handler_t = std::function<void(events_t)>;

class event_poller
{
public:
    event_poller();

    bool has_subscribers() const;
    void subscribe(int fd, events_t events = 0, event_handler_t * handler = nullptr);
    void unsubscribe(int fd);
    void modify(int fd, events_t events = 0, event_handler_t * handler = nullptr);

    std::pair<int, std::reference_wrapper<std::vector<epoll_event> const>> poll_events(std::optional<std::chrono::milliseconds> timeout = {});

private:
    event_poller(event_poller & other) = delete;
    event_poller(event_poller && other) = delete;

    std::vector<epoll_event> events_;
    safe_fd fd_;
};

class function_invoker
{
public:
    template<typename F>
    function_invoker(event_poller & event_poller, F function) :
        event_poller_(event_poller),
        handler_([this, f = std::move(function)](events_t) {
            event_poller_.modify(fds_[1]);
            f();
        })
    {
        using std::operator""s;

        int fds[2];
        int r = ::pipe(fds);
        if (r < 0)
            throw std::runtime_error("Failed to create pipe: "s + std::strerror(errno));

        fds_[0] = fds[0];
        fds_[1] = fds[1];
        event_poller_.subscribe(fds_[1]);
    }

    ~function_invoker();

    void schedule();

private:
    function_invoker(function_invoker & other) = delete;
    function_invoker(function_invoker && other) = delete;

    event_poller & event_poller_;
    event_handler_t handler_;
    safe_fd fds_[2];
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
    fd_event_notifier(fd_event_notifier & other) = delete;
    fd_event_notifier(fd_event_notifier && other) = delete;

    event_poller & event_poller_;
    std::optional<event_handler_t> event_handler_;
    int fd_;
};

} // End of namespace
