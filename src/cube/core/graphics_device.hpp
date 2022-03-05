#pragma once

#include <cube/specs.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/color.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/engine_context.hpp>
#include <cstring>

namespace cube::core
{

enum class graphics_fill_mode
{
    none,
    solid,
};

struct graphics_state
{
    enum dirty_flags : int
    {
        dirty_draw_color = 0x0001,
        dirty_fill_mode = 0x0002,
        dirty_all = 0xffff,
    };

    color draw_color{color_transparent};
    graphics_fill_mode fill_mode{graphics_fill_mode::solid};

    int dirty_flags{dirty_all};
};

struct graphics_buffer
{
    rgba_t data[cube_size_3d]{0};

    void operator=(graphics_buffer const & other)
    {
        std::memcpy(data, other.data, sizeof(data));
    }
};

inline void scale(graphics_buffer & buffer, double scalar)
{
    rgba_t * data = buffer.data;
    for (int i = 0; i < cube_size_3d; ++i)
        scale(*data++, scalar);
}

inline void blend(graphics_buffer const & lhs, graphics_buffer & rhs)
{
    rgba_t const * lhs_data = lhs.data;
    rgba_t * rhs_data = rhs.data;
    for (int i = 0; i < cube_size_3d; ++i)
        blend(*lhs_data++, *rhs_data++);
}

class engine_context;
class graphics_device
{
public:
    virtual ~graphics_device() = default;

    void update_state(graphics_state const & state);
    void draw(voxel_t const & voxel);
    void draw_sphere(voxel_t const & origin, int radius);
    void fill();

    void show_animation(std::shared_ptr<animation> animation);
    void render_animation(); // Render drawn voxels to the actual display

protected:
    graphics_device(engine_context & context);

    io_context_t & io_context();

    virtual int map_to_offset(int x, int y, int z) const;

private:

    graphics_device(graphics_device & other) = delete;
    graphics_device(graphics_device && other) = delete;

    virtual void show(graphics_buffer const & buffer) = 0;

    io_context_t & io_context_;
    graphics_buffer buffer_;
    graphics_fill_mode fill_mode_;
    color draw_color_;
    std::shared_ptr<animation> animation_;
};

} // End of namespace
