#include <cube/core/graphics_device.hpp>
#include <cube/core/math.hpp>
#include <chrono>

using namespace std::chrono;

namespace
{

constexpr cube::core::voxel_t voxel_begin{cube::cube_axis_min_value, cube::cube_axis_min_value, cube::cube_axis_min_value};
constexpr cube::core::voxel_t voxel_end{cube::cube_axis_max_value, cube::cube_axis_max_value, cube::cube_axis_max_value};
constexpr cube::core::range voxel_range{voxel_begin, voxel_end};

} // End of namespace

namespace cube::core
{

void graphics_device::render_time::update()
{
    uint64_t now = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
    uint64_t elapsed = now - nanos_previous;

    nanos_dt = nanos_dt - (nanos_dt >> 2) + (elapsed >> 2);
    nanos_previous = now;
}

nanoseconds graphics_device::avg_render_interval() const
{
    return nanoseconds(render_time_.nanos_dt);
}

void graphics_device::update_state(graphics_state const & state)
{
    if (state.dirty_flags & graphics_state::dirty_draw_color)
        draw_color_ = state.draw_color;
}

void graphics_device::draw(voxel_t const & voxel)
{
    if (visible(voxel)) {
        int const offset = map_to_offset(voxel.x, voxel.y, voxel.z);
        blend(draw_color_, buffer_.data[offset]);
    }
}

void graphics_device::line(voxel_t const & /* start */, voxel_t const & /* end */)
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

graphics_device::graphics_device(engine_context & context) :
    io_context_(context.io_context)
{ }

io_context_t & graphics_device::io_context()
{
    return io_context_;
}

int graphics_device::map_to_offset(int x, int y, int z) const
{
    return x + y * cube_size_1d + z * cube_size_2d;
}

} // End of namespace
