#include <core/graphics_device.h>

using namespace cube::core;

void graphics_device::show_animation(const animation::pointer &animation)
{
    animation_ = animation;
    if(nullptr != animation_) {
        animation_->init();
        animation_->paint_event(*this);
        refresh();
    }
}

void graphics_device::render_animation()
{
    if(nullptr != animation_ && animation_->dirty()) {
        animation_->paint_event(*this);
        refresh();
    }
}