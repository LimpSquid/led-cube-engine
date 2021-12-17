#pragma once

#include <boost/noncopyable.hpp>
#include <functional>
#include <chrono>
#include <memory>

namespace cube::core
{

using tick_handler_t = std::function<void(std::chrono::steady_clock::time_point, std::chrono::milliseconds)>;

class engine_context;
class tick_subscription :
    boost::noncopyable
{
public:
    using pointer = std::unique_ptr<tick_subscription>;

    tick_subscription(engine_context & context, std::chrono::milliseconds interval, tick_handler_t handler);
    ~tick_subscription();

    template<typename ... Args>
    static pointer create(Args && ... args)
    {
        return std::make_unique<tick_subscription>(std::forward<Args>(args)...);
    }

private:
    engine_context & context_;
    uint64_t const id_;
};

} // End of namespace
