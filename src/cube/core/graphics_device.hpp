#pragma once

#include <cube/specs.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/color.hpp>
#include <cube/core/voxel.hpp>
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
    rgba_t data[cube_size_3d];

    void operator=(graphics_buffer const & other)
    {
        memcpy(data, other.data, sizeof(data));
    }
};

class graphics_device
{
public:
    virtual ~graphics_device() = default;

    double fps() const;

    void update_state(graphics_state const & state);
    void draw(voxel_t const & voxel);
    void line(voxel_t const & start, voxel_t const & end);
    void fill();

    void show_animation(animation * animation);
    void render_animation(); // Render drawn voxels to the actual display
    void do_poll(); // Do a poll, which may block

protected:
    virtual int map_to_offset(int x, int y, int z) const;

private:
    struct render_time
    {
        uint64_t nanos_dt{1}; // just to avoid division by 0
        uint64_t nanos_previous{0};

        void update();
    };

    virtual void show(graphics_buffer const & buffer) = 0;
    virtual void poll() = 0;

    graphics_buffer buffer_;
    color draw_color_;
    animation * animation_;
    render_time render_time_;
};

} // End of namespace
