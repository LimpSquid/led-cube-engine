#pragma once

#include <core/graphics_device.h>

namespace cube::hal
{

class voxel_display : public core::graphics_device
{
public:
    voxel_display();
    virtual ~voxel_display() override;

private:
    virtual void draw_voxel(int x, int y, int z, const color_type &color) override;
    virtual void draw_line(int x1, int y1, int z1, int x2, int y2, int z2, const color_type &color) override;
    virtual void fill(const color_type &color) override;
    virtual void refresh() override;
};

}