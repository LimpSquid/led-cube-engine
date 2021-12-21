#include <cube/core/timers.hpp>
#include <cube/core/engine_context.hpp>

using namespace std::chrono;

namespace
{

uint64_t ticker_id = 0;

template<typename Container>
auto find_ticker(Container & tickers, uint64_t id)
{
    return std::find_if(tickers.begin(), tickers.end(),
        [id](auto const & ticker) { return ticker.id == id; });
}

} // End of namespace

namespace cube::core
{

recurring_timer::recurring_timer(engine_context & context, timer_handler_t handler) :
    context_(context),
    handler_(std::move(handler)),
    id_(ticker_id++)
{ }

recurring_timer::~recurring_timer()
{
    stop();
}

void recurring_timer::start(milliseconds interval, bool trigger_on_start)
{
    auto search = find_ticker(context_.tickers, id_);

    if (search == context_.tickers.end()) {
        auto const now = steady_clock::now();
        context_.tickers.push_back(engine_context::ticker{
            [this](auto now, auto elapsed) { handler_(std::move(now), std::move(elapsed)); },
            interval,
            now,
            trigger_on_start ? now : (now + interval),
            id_
        });
    }
}

void recurring_timer::restart()
{
    auto search = find_ticker(context_.tickers, id_);

    if (search != context_.tickers.end()) {
        auto const now = steady_clock::now();
        search->last = now;
        search->next = now + search->interval;
    }
}

void recurring_timer::stop()
{
    auto search = find_ticker(context_.tickers, id_);

    if (search != context_.tickers.end())
        context_.tickers.erase(search);
}

single_shot_timer::single_shot_timer(engine_context & context, timer_handler_t handler) :
    recurring_timer(context, [this, h = std::move(handler)](auto now, auto elapsed) {
        stop(); // stop before handler as the handler may restart the timer
        h(std::move(now), std::move(elapsed));
    })
{ }

} // End of namespace
