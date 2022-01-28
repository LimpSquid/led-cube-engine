#include <cube/core/painter.hpp>
#include <cube/core/color.hpp>

namespace cube::core
{

painter::painter(graphics_device & device) :
    device_(device)
{ }

painter::~painter()
{ }

void painter::set_color(color const & color)
{
    state_.draw_color = color;
    state_.dirty_flags |= graphics_state::dirty_draw_color;
}

void painter::set_fill_mode(graphics_fill_mode const & mode)
{
    state_.fill_mode = mode;
    state_.dirty_flags |= graphics_state::dirty_fill_mode;
}

void painter::draw(voxel_t const & v)
{
    update_state();
    device_.draw(v);
}

void painter::wipe_canvas()
{
    color const old = state_.draw_color;

    set_color(color_black);
    update_state();
    device_.fill();
    set_color(old);
}

void painter::fill_canvas()
{
    update_state();
    device_.fill();
}

void painter::sphere(voxel_t const & origin, int radius)
{
    if (radius <= 0)
        return draw(origin);

    update_state();
    device_.draw_sphere(origin, radius);
}

void painter::update_state()
{
    device_.update_state(state_);
    state_.dirty_flags = 0;
}

} // End of namespace
