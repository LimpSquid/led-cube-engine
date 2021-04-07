#pragma once

#include <core/animation.h>
#include <util/color.h>

namespace cube::core
{

class graphics_device
{
public:
    using color_type = util::basic_color<unsigned char>;

    virtual ~graphics_device() = default;
    virtual void draw_voxel(int x, int y, int z, const color_type &color) = 0;
    virtual void draw_line(int x1, int y1, int z1, int x2, int y2, int z2, const color_type &color) = 0;
    virtual void fill(const color_type &color) = 0;
    void show(const animation::pointer &animation);
    void render();

protected:
    graphics_device() = default;
    virtual void update() = 0;

private:
    animation::pointer animation_;
};

}