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

class animation_session
{
public:
    animation_session()
    { }

    ~animation_session()
    {
        if (animation_)
            animation_->finish();
    }

    animation_session(animation_session &) = delete;
    animation_session(animation_session &&) = delete;

    void set(std::shared_ptr<cube::core::animation> animation)
    {
        if (animation_)
            animation_->finish();
        if (animation)
            animation->init();
        animation_ = animation;
    }

    bool matches(std::shared_ptr<cube::core::animation> animation)
    {
        return animation_ == animation;
    }

    cube::core::animation & operator*()
    {
        return *animation_;
    }

    operator bool() const
    {
        return bool(animation_);
    }

private:
    std::shared_ptr<cube::core::animation> animation_;
};

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

constexpr milliseconds poll_timeout{std::clamp(cube::animation_scene_interval, 5ms, 50ms)};

} // End of namespace

namespace cube::core
{

engine_context & engine::context()
{
    return context_;
}

void engine::load(std::shared_ptr<animation> animation)
{
    animation_ = animation;
}

void engine::run()
{
    animation_session session;
    std::once_flag shutdown_flag;
    bool run = true;

    while (run) {
        // Poll tickers before the animation is rendered as it is common practice
        // that an animation is marked dirty from within a ticker handler
        poll(context_.tickers);

        if (stopping_) {
            std::call_once(shutdown_flag, [&]() {
                for (auto * signal : context_.shutdown_signals)
                    (*signal)();
            });
            run = !context_.shutdown_signals.empty();
        } else {
            // New animation
            if (!session.matches(animation_))
                session.set(animation_);

            // Render animation
            if (session)
                device_->render(*session);
        }

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
    }
}

void engine::stop()
{
    stopping_ = true;
}

} // End of namespace
