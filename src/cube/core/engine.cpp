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

engine::~engine()
{

}

void engine::load(const animation::pointer &animation)
{
    animation_ = animation;
    device_->show(animation);
}

void engine::process()
{
    const milliseconds tick_event_ms = 15ms; // @Todo: we should make this configurable?
    microseconds elapsed_us = 0us;
    microseconds tick_elapsed_us = 0us;
    microseconds time_step_elapsed_us = 0us;
    steady_clock::time_point now = steady_clock::now();
    steady_clock::time_point previous = now;

    for(;;) {
        // @Todo: lock?

        // Update time tracking
        now = steady_clock::now();
        elapsed_us = duration_cast<microseconds>(now - previous);
        previous += elapsed_us;

        tick_elapsed_us += elapsed_us;
        time_step_elapsed_us += elapsed_us;
        if(nullptr != animation_) {
            // Tick event
            if(tick_elapsed_us >= tick_event_ms) {
                animation_->tick_event(tick_elapsed_us);
                tick_elapsed_us = 0us;
            }

            // Time step event
            if(time_step_elapsed_us >= animation_->config().time_step_ms) {
                animation_->time_step_event();
                time_step_elapsed_us = 0us;
            }
        }

        // Render
        device_->render();
    }
}