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
    start();
}

void animation::update()
{
    dirty_ = true;
}

void animation::finish()
{
    dirty_ = false;
    stop();
}

void animation::paint_event(graphics_device & device)
{
    dirty_ = false;
    paint(device);
}

animation::animation(engine_context & context) :
    context_(context),
    dirty_(true)
{ }

void animation::start()
{ }

void animation::stop()
{ }

animation_scene::animation_scene(animation & animation, std::optional<scene_update_handler_t> handler) :
    timer_(animation.context(), [&animation, h = std::move(handler)](auto, auto elapsed) {
        if (h)
            (*h)(std::move(elapsed));
        animation.update();
    })
{ }

void animation_scene::start()
{
    timer_.start(animation_scene_interval);
}

void animation_scene::stop()
{
    timer_.stop();
}

} // End of namespace
