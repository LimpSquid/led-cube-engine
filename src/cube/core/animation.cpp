#include <core/animation.h>

using namespace cube::core;
using namespace std::chrono;

const animation_config &animation::config() const
{
    return config_;
}

bool animation::dirty() const
{
    return dirty_;
}

void animation::init()
{
    dirty_ = true;
    configure(config_);
}

void animation::update()
{
    dirty_ = true;
}

void animation::time_step_event()
{
    time_step();
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

animation::animation()
{
    dirty_ = true;
}

void animation::configure(animation_config &config)
{
    // Do nothing by default
}

void animation::time_step()
{
    // Do nothing by default
}