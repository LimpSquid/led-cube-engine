#pragma once

#include <cube/core/graphics_device.hpp>

namespace cube::core
{

class painter
{
public:
    painter(graphics_device & device);
    ~painter();

    void set_color(color const & color);
    void set_fill_mode(graphics_fill_mode const & mode);

    void draw(voxel_t const & voxel);
    void fill_canvas();
    void sphere(voxel_t const & origin, int radius);

private:
    void update_state();

    graphics_device & device_;
    graphics_state state_;
};

} // End of namespace
