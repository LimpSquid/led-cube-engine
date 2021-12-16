#include <cube/core/graphics_device.hpp>

namespace cube::core
{

void graphics_device::update_state(graphics_state & state)
{
    if (state.dirty_flags & graphics_state::dirty_draw_color)
        draw_color_ = state.draw_color;

    state.dirty_flags = 0;
}

void graphics_device::draw(voxel_t const & voxel)
{
    int const offset = map_to_offset(voxel.x, voxel.y, voxel.z);
    blend(draw_color_, buffer_.data[offset]);
}

void graphics_device::draw_line(voxel_t const & voxel1, voxel_t const & voxel2)
{
    // Draw a voxel into the graphics buffer
}

void graphics_device::fill()
{
    rgba_t * data = buffer_.data;
    for (int i = 0; i < cube_size_3d; ++i)
        blend(draw_color_, *data++);
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

int graphics_device::map_to_offset(int x, int y, int z) const
{
    return x + y * cube_size_1d + z * cube_size_2d;
}

} // End of namespace
