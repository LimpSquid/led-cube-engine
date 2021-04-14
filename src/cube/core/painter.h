#pragma once

#include <core/graphics_device.h>

namespace cube::core
{

class painter
{
public:
    using color_type = graphics_device::color_type;

    painter(graphics_device &device);
    ~painter();

    void draw(int x, int y, int z, const color_type &color);
    void wipe_canvas();
    void fill_canvas(const color_type &color);

private:
    graphics_device &device_;
};

}