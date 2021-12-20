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
    void draw(voxel_t const & voxel);
    void wipe_canvas();
    void fill_canvas();
    void scatter(voxel_t const & origin, double radius, bool smooth = true);

private:
    void update_state();

    graphics_device & device_;
    graphics_state state_;
};

} // End of namespace
