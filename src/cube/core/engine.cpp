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

constexpr milliseconds poll_timeout{10};

template<typename Container>
void poll(Container & tickers)
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
    std::shared_ptr<animation> animation;
    bool new_animation = false;

    for (;;) {
        // Poll tickers
        poll(context_.tickers);

        // Service animation on graphics device
        new_animation = (animation != animation_);
        animation = animation_;

        if (animation) {
            if (new_animation)
                device_->show_animation(animation);
            else
                device_->render_animation();
        }

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

} // End of namespace
