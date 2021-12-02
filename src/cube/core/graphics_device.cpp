#include <cube/core/graphics_device.h>
#include <cube/core/color.h>

using namespace cube::core;

void graphics_device::show_animation(animation * animation)
{
    animation_ = animation;
    if (animation_) {
        animation_->init();
        animation_->paint_event(*this);
        show();
    }
}

void graphics_device::render_animation()
{
    if (animation_ && animation_->dirty()) {
        animation_->paint_event(*this);
        show();
    }
}
