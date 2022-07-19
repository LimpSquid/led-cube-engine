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
void poll(T & tickers)
{
    auto const now = steady_clock::now();

    for (auto & ticker : tickers) {
        if (!ticker.suspended && now >= ticker.next) {
            auto const elapsed = duration_cast<milliseconds>(now - ticker.last);

            ticker.last = now;
            ticker.next += ticker.interval;
            ticker.handler(now, elapsed);
        }
    }
}

template<typename T = void>
void call_all() {}
template<typename T, typename ... O>
void call_all(T const & callable, O const & ... others)
{
    callable();
    call_all(others ...);
}

constexpr milliseconds poll_timeout{std::clamp(cube::animation_scene_interval, 5ms, 50ms)};

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

basic_engine::basic_engine(engine_context & context) :
   context_(context),
   stopping_(false)
{ }

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
        poll(context_.tickers);

        // Poll self
        poll_one(stopping_);

        // Poll asio and event_poller
        assert(!context_.io_context.stopped());
        if (context_.event_poller.has_subscribers()) {
            auto [size, events] = context_.event_poller.poll_events(poll_timeout);
            for (int i = 0; i < size; ++i)
                if (auto user_data = events.get().at(i).data.ptr)
                    (*reinterpret_cast<event_handler_t *>(user_data))(events.get().at(i).events);

            context_.io_context.poll(); // Run all asio ready handlers
        } else
            context_.io_context.run_one_for(poll_timeout); // No events, just poll asio

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
