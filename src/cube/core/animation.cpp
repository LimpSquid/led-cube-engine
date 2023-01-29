#include <cube/core/animation.hpp>
#include <cube/specs.hpp>

namespace cube::core
{

engine_context & animation::context()
{
    return context_;
}

animation_state animation::state() const
{
    return state_;
}

bool animation::dirty() const
{
    return dirty_;
}

void animation::init()
{
    dirty_ = true;
    scene_timer_.start(animation_scene_interval);
    change_state(animation_state::running);
}

void animation::update()
{
    dirty_ = true;
}

void animation:: about_to_finish()
{
    change_state(animation_state::stopping);
}

void animation::finish()
{
    dirty_ = false;
    scene_timer_.stop();
    change_state(animation_state::stopped);
}

void animation::paint_event(graphics_device & device)
{
    dirty_ = false;
    paint(device);
}

std::optional<double> animation::motion_blur() const
{
    return std::optional<double>{};
}

animation::animation(engine_context & context) :
    context_(context),
    scene_timer_(context, [&](auto, auto elapsed) {
        scene_tick(std::move(elapsed));
        update();
    }),
    state_(animation_state::stopped),
    dirty_(true)
{ }

void animation::change_state(animation_state state)
{
    if (state != state_) {
        state_ = state;
        state_changed(state);
    }
}

void animation::scene_tick(std::chrono::milliseconds)
{ }

void animation::state_changed(animation_state)
{ }

} // End of namespace
