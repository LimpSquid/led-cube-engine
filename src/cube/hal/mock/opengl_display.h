#pragma once

#include <cube/core/graphics_device.h>

namespace cube::hal
{

class opengl_display :
    public core::graphics_device
{
public:
    opengl_display();
    virtual ~opengl_display() override;

private:
    virtual void poll() override;
    virtual void draw_voxel(int x, int y, int z, core::color const & color) override;
    virtual void draw_line(int x1, int y1, int z1, int x2, int y2, int z2, core::color const & color) override;
    virtual void fill(core::color const & color) override;
    virtual void show() override;
};

}
