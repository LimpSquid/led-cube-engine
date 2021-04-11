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

    void wipe_canvas();
    void fill_canvas(const color_type &color);

private:
    graphics_device &device_;
};

}