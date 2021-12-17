#pragma once

#include <functional>
#include <chrono>

namespace cube::core
{

class engine;
class engine_context
{
    friend class tick_subscription;
    friend class engine;

    struct ticker
    {
        std::function<void(std::chrono::steady_clock::time_point, std::chrono::milliseconds)> handler;
        std::chrono::milliseconds interval;
        std::chrono::steady_clock::time_point last;
        std::chrono::steady_clock::time_point next;
        uint64_t id;
    };

    std::vector<ticker> tickers;
};

} // End of namespace
