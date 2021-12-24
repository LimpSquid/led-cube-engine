#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <functional>
#include <chrono>

namespace cube::core
{

using io_context_t = boost::asio::io_context;
using executor_work_guard_t = boost::asio::executor_work_guard<io_context_t::executor_type>;

class engine_context
{
    friend class recurring_timer;
    friend class engine;
    friend class graphics_device;

    struct ticker
    {
        std::function<void(
            std::chrono::steady_clock::time_point,
            std::chrono::milliseconds)> handler;
        std::chrono::milliseconds interval;
        std::chrono::steady_clock::time_point last;
        std::chrono::steady_clock::time_point next;
        uint64_t id;
    };

    std::vector<ticker> tickers;

    // asio
    io_context_t io_context;
    executor_work_guard_t work_guard{io_context.get_executor()};
};

} // End of namespace
