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

void engine::load(animation * animation)
{
    animation_ = animation;
}

void engine::run()
{
    animation * animation = nullptr;
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

        // Poll asio
        context_.io_context.run_one_for(10ms);
    }
}

} // End of namespace
