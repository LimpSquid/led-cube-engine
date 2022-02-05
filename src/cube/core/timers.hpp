#pragma once

#include <functional>
#include <chrono>
#include <memory>

namespace cube::core
{

using timer_handler_t = std::function<void(
    std::chrono::steady_clock::time_point /* now */,
    std::chrono::milliseconds /* elapsed */)>;

class engine_context;
class recurring_timer
{
public:
    recurring_timer(engine_context & context, timer_handler_t handler);
    ~recurring_timer();

    bool is_running() const;
    void start(std::chrono::milliseconds interval);
    void stop();

private:
    recurring_timer(recurring_timer & other) = delete;
    recurring_timer(recurring_timer && other) = delete;

    engine_context & context_;
    uint64_t const id_;
};

struct single_shot_timer :
    recurring_timer
{
    single_shot_timer(engine_context & context, timer_handler_t handler);
};

} // End of namespace
