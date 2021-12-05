#include <cube/core/engine.hpp>
#include <cube/core/graphics_device.hpp>
#include <cube/core/animation.hpp>
#include <stdexcept>
#include <chrono>
#include <thread>

using namespace cube::core;
using namespace std::chrono;
using namespace std::chrono_literals;

engine::engine(graphics_device *device) :
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
    milliseconds const tick_event_interval = 15ms;
    steady_clock::time_point now = steady_clock::now();
    steady_clock::time_point tick_tp = now;
    steady_clock::time_point time_step_tp = now;
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
            tick_tp += tick_event_interval;
            time_step_tp += animation->config().time_step_interval;

            device_->show_animation(animation);
            continue;
        }

        // Tick event
        if (now >= tick_tp) {
            animation->tick_event(tick_event_interval);
            tick_tp += tick_event_interval;
        }

        // Time step event
        if (now >= time_step_tp) {
            animation->time_step_event();
            time_step_tp += animation->config().time_step_interval;
        }

        // Finally render the animation and poll the device (may block)
        device_->render_animation();
        device_->do_poll();
    }
}
