#pragma once

#include <core/graphics_device.h>

namespace cube::core
{

class voxel_display : public graphics_device
{
public:
    voxel_display();
    virtual ~voxel_display() override;
};

}