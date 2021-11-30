#pragma once

#include <cube/core/graphics_device.h>

namespace cube::core
{

class painter
{
public:
    painter(graphics_device & device);
    ~painter();

    void draw(int x, int y, int z, color const & color);
    void wipe_canvas();
    void fill_canvas(color const & color);

private:
    graphics_device & device_;
};

}
