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

void painter::draw(int x, int y, int z, color const & color)
{
    device_.draw_voxel(x, y, z, color);
}

void painter::wipe_canvas()
{
    device_.fill({});
}

void painter::fill_canvas(color const & color)
{
    device_.fill(color);
}
