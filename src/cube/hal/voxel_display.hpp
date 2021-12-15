#pragma once

#include <cube/core/graphics_device.hpp>

namespace cube::hal
{

class voxel_display :
    public core::graphics_device
{
public:
    voxel_display();
    virtual ~voxel_display() override;

private:
    virtual void show(core::graphics_buffer const & buffer) override;
    virtual void poll() override;
};

} // End of namespace
