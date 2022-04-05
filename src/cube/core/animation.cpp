#include <cube/core/animation.hpp>
#include <cube/specs.hpp>

namespace cube::core
{

engine_context & animation::context()
{
    return context_;
}

bool animation::dirty() const
{
    return dirty_;
}

void animation::init()
{
    dirty_ = true;
    scene_timer_.start(animation_scene_interval);
    start();
}

void animation::update()
{
    dirty_ = true;
}

void animation::finish()
{
    dirty_ = false;
    scene_timer_.stop();
    stop();
}

void animation::paint_event(graphics_device & device)
{
    dirty_ = false;
    paint(device);
}

animation::animation(engine_context & context) :
    context_(context),
    scene_timer_(context, [&](auto, auto elapsed) {
        scene_tick(std::move(elapsed));
        update();
    }),
    dirty_(true)
{ }

void animation::start()
{ }

void animation::scene_tick(std::chrono::milliseconds)
{ }

void animation::stop()
{ }

} // End of namespace
