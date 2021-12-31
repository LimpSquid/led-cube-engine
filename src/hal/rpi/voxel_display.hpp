#pragma once

#include <cube/core/graphics_device.hpp>

namespace hal::rpi
{

class voxel_display :
    public cube::core::graphics_device
{
public:
    voxel_display(cube::core::engine_context & context);

private:
    virtual void show(cube::core::graphics_buffer const & buffer) override;
};

} // End of namespace

namespace hal { using graphics_device_t = rpi::voxel_display; }
