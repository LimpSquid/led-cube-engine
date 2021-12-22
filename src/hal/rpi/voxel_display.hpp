#pragma once

#include <cube/core/graphics_device.hpp>

namespace hal::rpi
{

class voxel_display :
    public cube::core::graphics_device
{
public:
    voxel_display();
    virtual ~voxel_display() override;

private:
    virtual void show(cube::core::graphics_buffer const & buffer) override;
    virtual void poll() override;
};

} // End of namespace

namespace hal { using graphics_device_t = rpi::voxel_display; }
