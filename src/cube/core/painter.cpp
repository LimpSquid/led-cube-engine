#include <cube/core/painter.hpp>
#include <cube/core/color.hpp>
#include <cassert>

using namespace cube::core;

painter::painter(graphics_device & device) :
    device_(device)
{

}

painter::~painter()
{

}

void painter::set_color(color const & color)
{
    state_.draw_color = color;
    state_.dirty_flags |= graphics_state::dirty_draw_color;
}

void painter::draw(voxel const & voxel)
{
    update_state();
    device_.draw_voxel(voxel.x, voxel.y, voxel.z);
}

void painter::wipe_canvas()
{
    color const old = state_.draw_color;

    set_color(color_clear);
    update_state();
    device_.fill();
    set_color(old);
}

void painter::fill_canvas()
{
    update_state();
    device_.fill();
}

void painter::update_state()
{
    device_.update_state(state_);
    assert(state_.dirty_flags == 0);
}
