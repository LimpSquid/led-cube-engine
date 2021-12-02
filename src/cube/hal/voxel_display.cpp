#include <cube/hal/voxel_display.h>
#include <cube/core/color.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace cube::core;
using namespace cube::hal;

voxel_display::voxel_display()
{

}

voxel_display::~voxel_display()
{

}

void voxel_display::poll()
{
    // Do other hardware stuff, like bus communication?
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

void voxel_display::show()
{
    // Send graphics buffer to slaves, swap frame buffers etc.
}
