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

template<typename TickerContainer>
void poll(TickerContainer & tickers)
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
    if (nullptr == device_)
        throw std::invalid_argument("Graphics device cannot be nullptr");
}

void engine::load(animation * animation)
{
    animation_ = animation;
}

void engine::run()
{
    steady_clock::time_point now = steady_clock::now();
    animation * animation = nullptr;
    bool init = false;

    for (;;) {
        init = (animation != animation_);
        animation = animation_;
        now = steady_clock::now();

        // Animation to be serviced?
        if (nullptr == animation) {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        // Init new animation
        if (init) {
            device_->show_animation(animation);
            continue;
        }

        poll(context_.tickers);

        // Finally render the animation and poll the device (may block)
        device_->render_animation();
        device_->do_poll();
    }
}

} // End of namespace
