#pragma once

#include <cube/specs.hpp>
#include <cube/core/color.hpp>
#include <cube/core/voxel.hpp>
#include <cube/core/engine_context.hpp>
#include <cstring>
#include <array>
#include <chrono>

namespace cube::core
{

namespace detail
{

template<typename T>
class flip_flop
{
public:
    using buffer_t = T;

    flip_flop() :
        f(&buffers_[0]),
        b(&buffers_[1])
    { }

    buffer_t & front() { return *f; }
    buffer_t & back() { return *b; }
    buffer_t * operator->() { return f; }
    buffer_t & operator*() { return *f; }
    void flip() { std::swap(f, b); }

    template<typename F>
    void flip_and_fill(F value)
    {
        flip();
        f->fill(value);
    }

private:
    buffer_t buffers_[2]; // FIXME: allocate on heap instead of stack?
    buffer_t * f;
    buffer_t * b;
};

} // End of namespace

enum class graphics_fill_mode
{
    none,
    solid,
};

struct graphics_state
{
    enum dirty_flags : unsigned int
    {
        dirty_draw_color = 0x0001,
        dirty_fill_mode = 0x0002,
        dirty_all = UINT_MAX,
    };

    color draw_color{color_transparent};
    graphics_fill_mode fill_mode{graphics_fill_mode::solid};

    unsigned int dirty_flags{dirty_all};
};

struct graphics_buffer
{
    std::array<rgba_t, cube_size_3d> data = {};

    constexpr static std::size_t size() { return cube_size_3d; }

    auto begin() { return data.begin(); }
    auto const begin() const { return data.begin(); }
    auto end() { return data.end(); }
    auto const end() const { return data.end(); }
    void fill(rgba_t value) { data.fill(value); }
};

inline void scale(graphics_buffer & buffer, double scalar)
{
    for (rgba_t & data : buffer)
        scale(data, scalar);
}

class engine_context;
class animation;
class graphics_device
{
public:
    virtual ~graphics_device() = default;

    void update_state(graphics_state const & state);
    void draw(voxel_t const & voxel);
    void draw_sphere(voxel_t const & origin, int radius);
    void fill();

    void render(animation & anim);

    // thread safe given that each thread accesses a different voxel
    void draw_with_color(voxel_t const & voxel, color const & color);

protected:
    graphics_device(engine_context & context);

    engine_context & context();

    virtual int map_to_offset(int x, int y, int z) const;

private:
    graphics_device(graphics_device &) = delete;
    graphics_device(graphics_device &&) = delete;

    void apply_motion_blur(double blur);
    virtual void show(graphics_buffer const & buffer) = 0;

    engine_context & context_;
    detail::flip_flop<graphics_buffer> buffer_;
    graphics_fill_mode fill_mode_;
    color draw_color_;
    std::chrono::steady_clock::time_point last_render_tp_;
    std::chrono::milliseconds render_time_acc_;
};

} // End of namespace
