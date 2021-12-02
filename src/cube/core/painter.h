#pragma once

#include <cube/core/graphics_device.h>

namespace cube::core
{

struct voxel
{
    int x;
    int y;
    int z;
};

class painter
{
public:
    painter(graphics_device & device);
    ~painter();

    void set_color(color const & color);
    void draw(voxel const & voxel);
    void wipe_canvas();
    void fill_canvas();

private:
    void update_state();

    graphics_device & device_;
    graphics_state state_;
};

}
