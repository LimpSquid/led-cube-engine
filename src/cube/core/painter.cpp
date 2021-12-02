#include <cube/core/painter.h>
#include <cube/core/color.h>

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
    update_state();
}

void painter::draw(voxel const & voxel)
{
    device_.draw_voxel(voxel.x, voxel.y, voxel.z);
}

void painter::wipe_canvas()
{
    color const old = state_.draw_color;
    set_color({});
    device_.fill();
    set_color(old);
}

void painter::fill_canvas()
{
    device_.fill();
}

void painter::update_state()
{
    device_.update_state(state_);
}
