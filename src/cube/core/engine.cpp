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
        if (now >= ticker.next) {
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

engine::engine(engine_context & context, graphics_device *device) :
    context_(context),
    device_(device),
    animation_(nullptr)
{
    if (!device_)
        throw std::invalid_argument("Graphics device cannot be nullptr");
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

        // Poll device (may block)
        device_->do_poll();

        // Service animation
        new_animation = (animation != animation_);
        animation = animation_;

        if (!animation)
            std::this_thread::sleep_for(100us); // Todo: eventually do not sleep, but wait until some event has happened
        else if (new_animation)
            device_->show_animation(animation);
        else
            device_->render_animation();
    }
}

} // End of namespace
