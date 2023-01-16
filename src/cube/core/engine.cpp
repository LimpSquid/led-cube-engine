#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/graphics_device.hpp>
#include <cube/core/animation.hpp>
#include <stdexcept>
#include <chrono>
#include <thread>

using namespace std::chrono;

namespace
{

template<typename T>
milliseconds poll(T & tickers)
{
    auto const now = high_resolution_clock::now();
    auto min_time_until_next = milliseconds::max();

    for (auto & ticker : tickers) {
        if (ticker.suspended)
            continue;

        if (now >= ticker.next) {
            auto const elapsed = round<milliseconds>(now - ticker.last);

            ticker.last = now;
            ticker.next += ticker.interval;
            ticker.handler(now, elapsed);

            min_time_until_next = std::min(min_time_until_next, ticker.interval);
        } else
            min_time_until_next = std::min(min_time_until_next, ceil<milliseconds>(ticker.next - now));
    }

    return min_time_until_next;
}

template<typename T = void>
void call_all() {}
template<typename T, typename ... O>
void call_all(T const & callable, O const & ... others)
{
    callable();
    call_all(others ...);
}

constexpr milliseconds default_poll_timeout{25};

} // End of namespace

namespace cube::core
{

namespace detail
{

animation_session::animation_session()
{ }

animation_session::~animation_session()
{
    if (animation_)
        animation_->finish();
}

void animation_session::set(std::shared_ptr<cube::core::animation> animation)
{
    if (animation_ == animation)
        return; // Already set
    if (animation_)
        animation_->finish();
    if (animation)
        animation->init();
    animation_ = animation;
}

cube::core::animation & animation_session::operator*()
{
    return *animation_;
}

animation_session::operator bool() const
{
    return bool(animation_);
}

} // End of namespace

engine_context & basic_engine::context()
{
    return context_;
}

void basic_engine::run()
{
    do_run();
}

void basic_engine::run_while(predicate_t predicate)
{
    std::once_flag stop_flag;
    do_run([&]() {
        if (!predicate())
            std::call_once(stop_flag, std::bind(&basic_engine::stop, this));
    });
}

void basic_engine::stop()
{
    stopping_ = true;
}

basic_engine::basic_engine(engine_context & context) :
    basic_engine(context, default_poll_timeout)
{ }

basic_engine::basic_engine(engine_context & context, milliseconds poll_timeout) :
   context_(context),
   poll_timeout_(poll_timeout),
   stopping_(false)
{ }

template<typename ... F>
void basic_engine::do_run(F ... extras)
{
    std::once_flag shutdown_flag;
    bool run = true;
    stopping_ = false;

    while (run) {
        // Poll extras, if any
        call_all(extras ...);

        // Poll tickers
        auto const time_until_next = poll(context_.tickers);
        auto const timeout = std::min(poll_timeout_, time_until_next);

        // Poll self
        poll_one(stopping_);

        // Poll asio and event_poller
        assert(!context_.io_context.stopped());
        if (context_.event_poller.has_subscribers()) {
            auto [size, events] = context_.event_poller.poll_events(timeout);
            for (int i = 0; i < size; ++i)
                if (auto user_data = events.get().at(i).data.ptr)
                    (*reinterpret_cast<event_handler_t *>(user_data))(events.get().at(i).events);

            context_.io_context.poll(); // Run all asio ready handlers
        } else
            context_.io_context.run_one_for(timeout); // No events, just poll asio

        // Finally check if we're stopping
        if (stopping_) {
            std::call_once(shutdown_flag, [&]() {
                for (auto * signal : context_.shutdown_signals)
                    (*signal)();
            });
            run = !context_.shutdown_signals.empty();
        }
    }
}

void render_engine::load(std::shared_ptr<animation> animation)
{
    animation_session_.set(animation);
}

render_engine::render_engine(engine_context & context, std::unique_ptr<graphics_device> && device) :
    basic_engine(context, default_poll_timeout),
    device_(std::move(device))
{ }

void render_engine::poll_one(bool stopping)
{
    // Render animation
    if (!stopping && animation_session_)
        device_->render(*animation_session_);
}

poll_engine::poll_engine(engine_context & context) :
    basic_engine(context)
{ }

void poll_engine::poll_one(bool)
{ }

} // End of namespace
