#include <core/engine.h>
#include <core/graphics_device.h>
#include <core/animation.h>
#include <stdexcept>
#include <chrono>

using namespace cube::core;
using namespace std::chrono;
using namespace std::chrono_literals;

engine::engine(graphics_device *device) :
    device_(device),
    thread_(&engine::process, this)
{
    if(nullptr == device_)
        throw std::invalid_argument("Graphics device cannot be nullptr");
}

void engine::load(const animation::pointer &animation)
{
    std::lock_guard<std::mutex> guard(animation_lock_);
    animation_ = animation;
}

void engine::process()
{
    const milliseconds tick_event_ms = 15ms;
    microseconds elapsed_us = 0us;
    microseconds tick_elapsed_us = 0us;
    microseconds time_step_elapsed_us = 0us;
    steady_clock::time_point now = steady_clock::now();
    steady_clock::time_point previous = now;
    animation::pointer animation;
    bool init = false;

    for(;;) {
        {
            std::lock_guard<std::mutex> guard(animation_lock_);
            init = (animation != animation_);
            animation = animation_;
        }

        // Animation to be serviced?
        if(nullptr == animation) {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        // Init new animation
        if(init) {
            previous = steady_clock::now();
            tick_elapsed_us = 0us;
            time_step_elapsed_us = 0us;

            device_->show_animation(animation);
            continue;
        }

        // Update time tracking
        now = steady_clock::now();
        elapsed_us = duration_cast<microseconds>(now - previous);
        previous += elapsed_us;

        // Tick event
        tick_elapsed_us += elapsed_us;
        if(tick_elapsed_us >= tick_event_ms) {
            animation->tick_event(tick_elapsed_us);
            tick_elapsed_us = 0us;
        }

        // Time step event
        time_step_elapsed_us += elapsed_us;
        if(time_step_elapsed_us >= animation->config().time_step_ms) {
            animation->time_step_event();
            time_step_elapsed_us = 0us;
        }

        // Finally render to device
        device_->render_animation();
    }
}