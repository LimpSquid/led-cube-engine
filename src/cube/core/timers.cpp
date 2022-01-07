#include <cube/core/timers.hpp>
#include <cube/core/engine_context.hpp>

using namespace std::chrono;

namespace
{

uint64_t ticker_id = 0;

template<typename Container>
auto & find_ticker_or_throw(Container & tickers, uint64_t id)
{
    auto search = std::find_if(tickers.begin(), tickers.end(),
        [id](auto const & ticker) { return ticker.id == id; });

    if (search == tickers.end())
        throw std::runtime_error("Unable to find tiker with id: " + std::to_string(id));
    return *search;
}

template<typename Container>
void remove_ticker(Container & tickers, uint64_t id)
{
    auto begin = std::remove_if(tickers.begin(), tickers.end(),
        [id](auto const & ticker) { return ticker.id == id; });

    tickers.erase(begin, tickers.end());
}

} // End of namespace

namespace cube::core
{

recurring_timer::recurring_timer(engine_context & context, timer_handler_t handler) :
    context_(context),
    id_(ticker_id++)
{
    context_.tickers.push_back(engine_context::ticker{
        [h = std::move(handler)](auto now, auto elapsed) { h(std::move(now), std::move(elapsed)); },
        {/* interval */}, {/* last */}, { /* next */ },
        id_,
        true
    });
}

recurring_timer::~recurring_timer()
{
    remove_ticker(context_.tickers, id_);
}

bool recurring_timer::is_running() const
{
    return !find_ticker_or_throw(context_.tickers, id_).suspended;
}

void recurring_timer::start(milliseconds interval)
{
    auto & ticker = find_ticker_or_throw(context_.tickers, id_);
    auto const now = steady_clock::now();

    ticker.interval = interval;
    ticker.last = now;
    ticker.next = now + interval;
    ticker.suspended = false;
}

void recurring_timer::stop()
{
    find_ticker_or_throw(context_.tickers, id_).suspended = true;
}

single_shot_timer::single_shot_timer(engine_context & context, timer_handler_t handler) :
    recurring_timer(context, [this, h = std::move(handler)](auto now, auto elapsed) {
        stop(); // stop before handler as the handler may restart the timer
        h(std::move(now), std::move(elapsed));
    })
{ }

} // End of namespace
