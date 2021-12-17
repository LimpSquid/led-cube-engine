#include <cube/core/subscriptions.hpp>
#include <cube/core/engine_context.hpp>
#include <algorithm>

using namespace std::chrono;

namespace
{

uint64_t subscription = 0;

} // End of namespace

namespace cube::core
{

tick_subscription::tick_subscription(
    engine_context & context,
    std::chrono::milliseconds interval,
    tick_handler_t handler,
    bool trigger_on_start) :
    context_(context),
    id_(subscription++)
{
    auto const now = steady_clock::now();

    context_.tickers.push_back(engine_context::ticker{
        [h=std::move(handler)](auto now, auto elapsed) { h(std::move(now), std::move(elapsed)); },
        interval,
        now,
        trigger_on_start ? now : (now + interval),
        id_
    });
}

tick_subscription::~tick_subscription()
{
    auto search = std::find_if(context_.tickers.begin(), context_.tickers.end(),
        [this](auto const & ticker) { return ticker.id == id_; });

    if (search != context_.tickers.end())
        context_.tickers.erase(search);
}

} // End of namespace
