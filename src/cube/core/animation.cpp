#include <cube/core/animation.hpp>

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
    configure();
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

void animation::configure()
{ }

void animation::stop()
{ }

} // End of namespace
