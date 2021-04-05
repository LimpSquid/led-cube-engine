#pragma once

#include <core/graphics_device.h>
#include <util/color.h>

namespace cube::hal
{

class voxel_display : public core::graphics_device
{
public:
    voxel_display();
    virtual ~voxel_display() override;

private:
    virtual void draw_voxel(int x, int y, int z, const util::color_uchar &color) override;
    virtual void update() override;
};

}