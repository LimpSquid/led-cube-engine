#pragma once

#include <core/animation.h>

namespace cube::util
{
class color;
}

namespace cube::core
{

class graphics_device
{
public:
    virtual ~graphics_device() = default;
    virtual void draw_voxel(int x, int y, int z, const util::color &color) = 0;
    void show(const animation::pointer &animation);
    void render();

protected:
    graphics_device() = default;
    virtual void update() = 0;

private:
    animation::pointer animation_;
};

}