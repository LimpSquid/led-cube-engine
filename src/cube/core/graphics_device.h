#pragma once

#include <cube/core/animation.h>

namespace cube::core
{

struct color;

class graphics_device
{
public:
    virtual ~graphics_device() = default;
    virtual void draw_voxel(int x, int y, int z, color const & color) = 0; // Draw a voxel
    virtual void draw_line(int x1, int y1, int z1, int x2, int y2, int z2, const color &color) = 0; // Draw a voxel line
    virtual void fill(color const & color) = 0; // Fill all voxels
    virtual void show() = 0; // Show drawn voxels on the actual display

    virtual void poll() = 0; // Called by engine's loop, may block

    void show_animation(animation * animation);
    void render_animation();

protected:
    graphics_device() = default;

private:
    animation * animation_;
};

}
