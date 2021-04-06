#pragma once

#include <core/graphics_device.h>

namespace cube::core
{
class painter
{
public:
    painter(graphics_device &device);
    ~painter();

private:
    graphics_device &device_;
    graphics_device_state state_;
};

}