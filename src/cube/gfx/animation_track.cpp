#include <cube/gfx/animation_track.hpp>

using namespace cube::core;
using namespace std::chrono;

namespace cube::gfx
{

animation_track::animation_state animation_track::state() const
{
    return state_;
}

bool animation_track::is_stopped() const
{
    return state_ == stopped;
}

bool animation_track::is_paused() const
{
    return state_ == paused;
}

bool animation_track::is_running() const
{
    return state_ == running;
}

bool animation_track::is_finished() const
{
    return state_ == finished;
}

milliseconds animation_track::time_remaining() const
{
    return time_;
}

milliseconds animation_track::time_total() const
{
    return read_property<milliseconds>(animation_time_ms, 10s);
}

void animation_track::start()
{
    switch (state_) {
        case stopped:
            time_ = time_total();
            [[fallthrough]];
        case paused:
            set_state(running);
            break;
        default:;
    }
}

void animation_track::pause()
{
    switch (state_) {
        case running:
            set_state(paused);
            break;
        default:;
    }
}

void animation_track::stop()
{
    switch (state_) {
        case paused:
        case running:
            set_state(stopped);
            break;
        default:;
    }
}

void animation_track::write_properties(std::vector<std::pair<property_label_type, property_value>> const & properties)
{
    for (auto const & property : properties)
        write_property(property.first, property.second);
}

animation_track::animation_track(engine_context & context) :
    context_(context),
    tick_sub_(context, 100ms, [this](auto, auto elapsed) { tick(elapsed); }),
    state_(stopped)
{ }

engine_context & animation_track::context()
{
    return context_;
}

void animation_track::state_changed(animation_state const &)
{ }

void animation_track::set_state(animation_state const & value)
{
    if (state_ != value) {
        state_ = value;
        state_changed(state_);
    }
}

void animation_track::tick(milliseconds const & elapsed)
{
    switch (state_) {
        case running:
            if (elapsed < time_)
                time_ -= elapsed;
            else {
                time_ = 0ms;
                set_state(finished);
            }
            break;
        default:;
    }
}

} // End of namespace
