#include <core/painter.h>

using namespace cube::core;

painter::painter(graphics_device &device) :
    device_(device)
{

}

painter::~painter()
{

}

void painter::draw(int x, int y, int z, const color_type &color)
{
    device_.draw_voxel(x, y, z, color);
}

void painter::wipe_canvas()
{
    device_.fill({ 0, 0, 0 });
}

void painter::fill_canvas(const color_type &color)
{
    device_.fill(color);
}