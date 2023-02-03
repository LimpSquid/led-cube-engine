#pragma once

#include <cube/core/events.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <functional>
#include <chrono>
#include <unordered_set>

namespace cube::core
{

using event_poller_t = event_poller;
using io_context_t = boost::asio::io_context;
using executor_work_guard_t = boost::asio::executor_work_guard<io_context_t::executor_type>;

class engine_shutdown_signal;
class engine_context
{
public:
    io_context_t io_context;
    event_poller_t event_poller;

private:
    // Following classes may access the internals of the engine's context
    friend class recurring_timer;
    friend class basic_engine;
    friend class engine_shutdown_signal;
    friend class graphics_device;

    executor_work_guard_t work_guard{io_context.get_executor()};

    struct ticker
    {
        std::function<void(
            std::chrono::high_resolution_clock::time_point,
            std::chrono::milliseconds)> handler;
        std::chrono::milliseconds interval;
        std::chrono::high_resolution_clock::time_point last;
        std::chrono::high_resolution_clock::time_point next;
        uint64_t id;
        bool suspended;
    };

    std::vector<std::shared_ptr<ticker>> tickers;
    std::unordered_set<engine_shutdown_signal *> shutdown_signals;
};

class engine_shutdown_signal
{
public:
    engine_shutdown_signal(engine_context & context) :
        context_(&context)
    {
        context_->shutdown_signals.insert(this);
    }

    ~engine_shutdown_signal()
    {
        remove();
    }

    void operator()()
    {
        shutdown_requested();
    }

protected:
    void ready_for_shutdown()
    {
        remove();
    }

private:
    engine_shutdown_signal(engine_shutdown_signal const &) = delete;
    engine_shutdown_signal(engine_shutdown_signal &&) = delete;

    virtual void shutdown_requested() = 0;

    void remove()
    {
        if (context_) {
            context_->shutdown_signals.erase(this);
            context_ = nullptr;
        }
    }

    engine_context * context_;
};

} // End of namespace
