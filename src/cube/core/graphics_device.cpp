#include <core/graphics_device.h>

using namespace cube::core;

void graphics_device::show(const animation::pointer &animation)
{
    animation_ = animation;
    if(nullptr != animation_)
        animation_->paint_event(*this);
}

void graphics_device::render()
{
    if(nullptr != animation_ && animation_->dirty())
        animation_->paint_event(*this);
}