#pragma once

#include <cube/specs.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/color.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/engine_context.hpp>
#include <cstring>
#include <array>

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
    std::array<rgba_t, cube_size_3d> data = {};

    auto begin() { return data.begin(); }
    auto const begin() const { return data.begin(); }
    auto end() { return data.end(); }
    auto const end() const { return data.end(); }
};

inline void scale(graphics_buffer & buffer, double scalar)
{
    for (rgba_t & data : buffer)
        scale(data, scalar);
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
