#include <core/graphics_device.h>

using namespace cube::core;

void graphics_device::load(const animation::pointer &animation)
{
    animation_ = animation;
    if(animation_)
        animation_->paint(*this);
}

void graphics_device::refresh()
{
    if(animation_)
        animation_->paint(*this);
}