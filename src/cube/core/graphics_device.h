#pragma once

#include <core/animation.h>
#include <util/brush.h>
#include <util/pen.h>

namespace cube::core
{

struct graphics_device_state
{
    util::brush brush;
    util::pen pen;
};

class graphics_device
{
public:
    virtual ~graphics_device() = default;
    virtual void draw_voxel(int x, int y, int z) = 0;
    virtual void draw_line(int x1, int y1, int z1, int x2, int y2, int z2) = 0;
    virtual void update_state(const graphics_device_state &state) = 0;
    void show(const animation::pointer &animation);
    void render();

protected:
    graphics_device() = default;
    virtual void update() = 0;

private:
    animation::pointer animation_;
    graphics_device_state state_;
};

}