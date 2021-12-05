#include <cube/core/graphics_device.hpp>

using namespace cube::core;

void graphics_device::update_state(graphics_state const & state)
{
    if (state.dirty_flags & graphics_state::dirty_draw_color)
        draw_color_ = state.draw_color;
}

void graphics_device::draw_voxel(int x, int y, int z)
{
    // Draw a voxel into the graphics buffer
}

void graphics_device::draw_line(int x1, int y1, int z1, int x2, int y2, int z2)
{
    // Draw a voxel into the graphics buffer
}

void graphics_device::fill()
{
    // Fill all the voxels of the graphics buffer

    // Todo: purely for testing remove eventually
    buffer_.test_color = draw_color_;
}

void graphics_device::show_animation(animation * animation)
{
    animation_ = animation;
    if (animation_) {
        animation_->init();
        animation_->paint_event(*this);
        show(buffer_);
    }
}

void graphics_device::render_animation()
{
    if (animation_ && animation_->dirty()) {
        animation_->paint_event(*this);
        show(buffer_);
    }
}

void graphics_device::do_poll()
{
    poll();
}
