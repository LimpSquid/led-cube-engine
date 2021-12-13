#pragma once

#include <cube/specs.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/color.hpp>
#include <cstring>

namespace cube::core
{

struct graphics_state
{
    enum dirty_flags : int
    {
        dirty_draw_color = 0x0001,
    };

    color draw_color;

    int dirty_flags;
};

struct graphics_buffer
{
    argb_t data[cube_size_3d];

    void operator=(graphics_buffer const & other)
    {
        memcpy(data, other.data, sizeof(data));
    }
};

class graphics_device
{
public:
    virtual ~graphics_device() = default;

    void update_state(graphics_state & state);
    void draw_voxel(int x, int y, int z);
    void draw_line(int x1, int y1, int z1, int x2, int y2, int z2);
    void fill();
    void translate(int dx, int dy, int dz);

    void show_animation(animation * animation);
    void render_animation(); // Render drawn voxels to the actual display
    void do_poll(); // Do a poll, which may block

protected:
    virtual int map_to_offset(int x, int y, int z) const;

private:
    virtual void show(graphics_buffer const & buffer) = 0;
    virtual void poll() = 0;

    graphics_buffer buffer_;
    color draw_color_;
    animation * animation_;
};

}
