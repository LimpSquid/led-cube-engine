#include <cube/core/graphics_device.hpp>
#include <glm/glm.hpp>
#include <algorithm>
#include <chrono>

using namespace std::chrono;

namespace cube::core
{

void graphics_device::render_time::update()
{
    uint64_t now = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
    uint64_t elapsed = now - nanos_previous;

    nanos_dt = nanos_dt - (nanos_dt >> 2) + (elapsed >> 2);
    nanos_previous = now;
}

double graphics_device::fps() const
{
    return (1000000000.0 / render_time_.nanos_dt);
}

void graphics_device::update_state(graphics_state const & state)
{
    if (state.dirty_flags & graphics_state::dirty_draw_color)
        draw_color_ = state.draw_color;
}

void graphics_device::draw(voxel_t const & voxel)
{
    int const offset = map_to_offset(voxel.x, voxel.y, voxel.z);
    blend(draw_color_, buffer_.data[offset]);
}

void graphics_device::line(voxel_t const & start, voxel_t const & end)
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
    // Finish old animation
    if (animation_)
        animation_->finish();

    // Init new animation
    if (animation) {
        animation_ = animation;
        animation->init();
        animation->paint_event(*this);
        show(buffer_);
    }
}

void graphics_device::render_animation()
{
    if (animation_ && animation_->dirty()) {
        animation_->paint_event(*this);
        show(buffer_);
    }

    render_time_.update();
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
