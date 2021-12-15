#include <cube/core/animation.hpp>

using namespace std::chrono;

namespace cube::core
{

animation_config const & animation::config() const
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

void animation::tick_event(microseconds const & interval)
{
    tick(interval);
}

void animation::paint_event(graphics_device & device)
{
    dirty_ = false;
    paint(device);
}

animation::animation()
{
    dirty_ = true;
}

void animation::configure(animation_config & config)
{
    // Do nothing by default
}

void animation::time_step()
{
    // Do nothing by default
}

} // End of namespace
