#include <core/animation.h>

using namespace cube::core;
using namespace std::chrono;

bool animation::dirty() const
{
    return dirty_;
}

void animation::update()
{
    dirty_ = true;
}

animation::animation()
{
    dirty_ = true;
}

void animation::tick_event(const microseconds &interval)
{
    tick(interval);
}

void animation::paint_event(graphics_device &device)
{
    dirty_ = false;
    paint(device);
}