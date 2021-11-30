#include <cube/hal/voxel_display.h>
#include <cube/core/color.h>

using namespace cube::core;
using namespace cube::hal;

voxel_display::voxel_display()
{

}

voxel_display::~voxel_display()
{

}

void voxel_display::draw_voxel(int x, int y, int z, color const & color)
{
    // Draw a voxel into the graphics buffer
}

void voxel_display::draw_line(int x1, int y1, int z1, int x2, int y2, int z2, color const & color)
{
    // Draw a voxel into the graphics buffer
}

void voxel_display::fill(color const & color)
{
    // Fill all the voxels of the graphics buffer
}

void voxel_display::refresh()
{
    // Send graphics buffer to slaves, swap frame buffers etc.
}
