#include <hal/voxel_display.h>

using namespace cube::hal;

voxel_display::voxel_display()
{

}

voxel_display::~voxel_display()
{

}

void voxel_display::draw_voxel(int x, int y, int z, const color_type &color)
{
    // Draw a voxel into the graphics buffer
}

void voxel_display::draw_line(int x1, int y1, int z1, int x2, int y2, int z2, const color_type &color)
{
    // Draw a voxel into the graphics buffer
}

void voxel_display::fill(const color_type &color)
{
    // Fill all the voxels of the graphics buffer
}

void voxel_display::refresh()
{
    // Send graphics buffer to slaves, swap frame buffers etc.
}