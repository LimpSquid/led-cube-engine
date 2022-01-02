#include <cube/core/graphics_device.hpp>
#include <cube/core/math.hpp>
#include <chrono>

using namespace std::chrono;

namespace
{

constexpr cube::core::voxel_t voxel_begin{cube::cube_axis_min_value, cube::cube_axis_min_value, cube::cube_axis_min_value};
constexpr cube::core::voxel_t voxel_end{cube::cube_axis_max_value, cube::cube_axis_max_value, cube::cube_axis_max_value};
constexpr cube::core::range voxel_range{voxel_begin, voxel_end};

} // End of namespace

namespace cube::core
{

void graphics_device::render_time::update()
{
    uint64_t now = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
    uint64_t elapsed = now - nanos_previous;

    nanos_dt = nanos_dt - (nanos_dt >> 2) + (elapsed >> 2);
    nanos_previous = now;
}

nanoseconds graphics_device::avg_render_interval() const
{
    return nanoseconds(render_time_.nanos_dt);
}

void graphics_device::update_state(graphics_state const & state)
{
    if (state.dirty_flags & graphics_state::dirty_draw_color)
        draw_color_ = state.draw_color;
    if (state.dirty_flags & graphics_state::dirty_fill_mode)
        fill_mode_ = state.fill_mode;
}

void graphics_device::draw(voxel_t const & voxel)
{
    draw_with_color(voxel, draw_color_);
}

void graphics_device::draw_sphere(voxel_t const & origin, int radius)
{
    auto const draw_shell = [this, origin](int radius, color const & c) {
        glm::dvec3 const box{radius, radius, radius};
        glm::dvec3 const min = glm::dvec3(origin) - box;
        glm::dvec3 const max = glm::dvec3(origin) + box;

        int const x_from = std::max(0, static_cast<int>(std::round(min.x)));
        int const y_from = std::max(0, static_cast<int>(std::round(min.y)));
        int const z_from = std::max(0, static_cast<int>(std::round(min.z)));
        int const x_to = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.x))) + 1;
        int const y_to = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.y))) + 1;
        int const z_to = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.z))) + 1;

        int const rr = radius * radius;
        int x, y, z;
        int xr, yr, zr;
        int xx, yy, zz;
        int cdx, cdy, cdz;
        int fdx, fdy, fdz;
        double dx, dy, dz;

        for (x = x_from, xr = x - origin.x, xx = xr * xr; x < x_to; x++, xr++, xx = xr * xr) {
            for (y = y_from, yr = y - origin.y, yy = yr * yr; y < y_to; y++, yr++, yy = yr * yr) {
                zz = rr - xx - yy;
                if (zz >= 0) {
                    dz = std::sqrt(static_cast<double>(zz));
                    cdz = static_cast<int>(std::ceil(dz));
                    fdz = static_cast<int>(std::floor(dz));
                    draw_with_color({x, y, origin.z - cdz}, c);
                    draw_with_color({x, y, origin.z + cdz}, c);
                    draw_with_color({x, y, origin.z - fdz}, c);
                    draw_with_color({x, y, origin.z + fdz}, c);
                }
            }
        }

        for (x = x_from, xr = x - origin.x, xx = xr * xr; x < x_to; x++, xr++, xx = xr * xr) {
            for (z = z_from, zr = z - origin.z, zz = zr * zr; z < z_to; z++, zr++, zz = zr * zr) {
                yy = rr - xx - zz;
                if (yy >= 0) {
                    dy = std::sqrt(static_cast<double>(yy));
                    cdy = static_cast<int>(std::ceil(dy));
                    fdy = static_cast<int>(std::floor(dy));
                    draw_with_color({x, origin.y - cdy, z}, c);
                    draw_with_color({x, origin.y + cdy, z}, c);
                    draw_with_color({x, origin.y - fdy, z}, c);
                    draw_with_color({x, origin.y + fdy, z}, c);
                }
            }
        }

        for (y = y_from, yr = y - origin.y, yy = yr * yr; y < y_to; y++, yr++, yy = yr * yr) {
            for (z = z_from, zr = z - origin.z, zz = zr * zr; z < z_to; z++, zr++, zz = zr * zr) {
                xx = rr - zz - yy;
                if (xx >= 0) {
                    dx = std::sqrt(static_cast<double>(xx));
                    cdx = static_cast<int>(std::ceil(dx));
                    fdx = static_cast<int>(std::floor(dx));
                    draw_with_color({origin.x - cdx, y, z}, c);
                    draw_with_color({origin.x + cdx, y, z}, c);
                    draw_with_color({origin.x - fdx, y, z}, c);
                    draw_with_color({origin.x + fdx, y, z}, c);
                }
            }
        }
    };

    switch (fill_mode_) {
        default:
        case graphics_fill_mode::solid:
            for(int r = 0; r <= radius; r++) {
                double scalar = std::min(1.0, 0.3 + (1.0 - static_cast<double>(r) / radius));
                draw_shell(r, draw_color_.vec() * scalar);
            }
            break;
        case graphics_fill_mode::none:
            draw_shell(radius, draw_color_);
            break;
    }
}

void graphics_device::fill()
{
    rgba_t * data = buffer_.data;
    for (int i = 0; i < cube_size_3d; ++i)
        blend(draw_color_, *data++);
}

void graphics_device::show_animation(animation * animation)
{
    // Finish old animation
    if (animation_)
        animation_->finish();

    // Init new animation
    if (animation) {
        animation_ = animation;
        animation->init();
        animation->paint_event(*this);
        show(buffer_);
    }
}

void graphics_device::render_animation()
{
    if (animation_ && animation_->dirty()) {
        animation_->paint_event(*this);
        show(buffer_);
    }

    render_time_.update();
}

graphics_device::graphics_device(engine_context & context) :
    io_context_(context.io_context),
    fill_mode_(graphics_fill_mode::solid),
    draw_color_(color_transparent)
{ }

io_context_t & graphics_device::io_context()
{
    return io_context_;
}

int graphics_device::map_to_offset(int x, int y, int z) const
{
    return x + y * cube_size_1d + z * cube_size_2d;
}

void graphics_device::draw_with_color(voxel_t const & voxel, color const & c)
{
    if (visible(voxel)) {
        int const offset = map_to_offset(voxel.x, voxel.y, voxel.z);
        blend(c, buffer_.data[offset]);
    }
}

} // End of namespace
