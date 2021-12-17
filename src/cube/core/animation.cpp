#include <cube/core/animation.hpp>

namespace cube::core
{

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

void animation::paint_event(graphics_device & device)
{
    dirty_ = false;
    paint(device);
}

animation::animation() :
    dirty_(true)
{ }

void animation::configure()
{ }

} // End of namespace
