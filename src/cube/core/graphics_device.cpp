#include <cube/core/graphics_device.hpp>
#include <cube/core/math.hpp>
#include <cube/core/parallel.hpp>
#include <3rdparty/glm/geometric.hpp>
#include <chrono>

using namespace std::chrono;

namespace cube::core
{

void graphics_device::render_time::update()
{
    auto const now = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
    auto const elapsed = now - nanos_previous;

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
    if (visible(voxel)) {
        int const offset = map_to_offset(voxel.x, voxel.y, voxel.z);
        blend(draw_color_, buffer_.data[offset]);
    }
}

void graphics_device::draw_sphere(voxel_t const & origin, int radius)
{
    glm::ivec3 const box{radius, radius, radius};
    glm::ivec3 const min = glm::ivec3(origin) - box;
    glm::ivec3 const max = glm::ivec3(origin) + box;

    int const x_from = std::max(0, min.x);
    int const y_from = std::max(0, min.y);
    int const z_from = std::max(0, min.z);
    int const x_to = std::min(cube_axis_max_value, max.x) + 1;
    int const y_to = std::min(cube_axis_max_value, max.y) + 1;
    int const z_to = std::min(cube_axis_max_value, max.z) + 1;

    auto const draw_shell = [&]() {
        int const rr = radius * radius;
        int x, y, z;
        int xr, yr, zr;
        int xx, yy, zz;
        int dx, dy, dz;

        for (x = x_from, xr = x - origin.x, xx = xr * xr; x < x_to; x++, xr++, xx = xr * xr) {
            for (y = y_from, yr = y - origin.y, yy = yr * yr; y < y_to; y++, yr++, yy = yr * yr) {
                zz = rr - xx - yy;
                if (zz >= 0) {
                    dz = static_cast<int>(std::ceil(std::sqrt(zz)));
                    draw({x, y, origin.z - dz});
                    draw({x, y, origin.z + dz});
                }
            }
        }

        for (x = x_from, xr = x - origin.x, xx = xr * xr; x < x_to; x++, xr++, xx = xr * xr) {
            for (z = z_from, zr = z - origin.z, zz = zr * zr; z < z_to; z++, zr++, zz = zr * zr) {
                yy = rr - xx - zz;
                if (yy >= 0) {
                    dy = static_cast<int>(std::ceil(std::sqrt(yy)));
                    draw({x, origin.y - dy, z});
                    draw({x, origin.y + dy, z});
                }
            }
        }

        for (y = y_from, yr = y - origin.y, yy = yr * yr; y < y_to; y++, yr++, yy = yr * yr) {
            for (z = z_from, zr = z - origin.z, zz = zr * zr; z < z_to; z++, zr++, zz = zr * zr) {
                xx = rr - zz - yy;
                if (xx >= 0) {
                    dx = static_cast<int>(std::ceil(std::sqrt(xx)));
                    draw({origin.x - dx, y, z});
                    draw({origin.x + dx, y, z});
                }
            }
        }
    };

    auto const draw_solid = [&]() {
        double r, scalar;
        int x, y, z, offset;

        for (x = x_from; x < x_to; x++) {
            for (y = y_from; y < y_to; y++) {
                for (z = z_from; z < z_to; z++) {
                    r = glm::length(glm::dvec3(x, y, z) - glm::dvec3(origin));
                    if (less_than_or_equal(r, static_cast<double>(radius))) {
                        // we use an offset to only scale the color after
                        // (r / radius) reaches a certain threshold
                        scalar = std::min(1.0, 0.3 + (1.0 - r / radius));
                        offset = map_to_offset(x, y, z);
                        blend(draw_color_.vec() * scalar, buffer_.data[offset]);
                    }
                }
            }
        }
    };

    switch (fill_mode_) {
        default:
        case graphics_fill_mode::solid: draw_solid();   break;
        case graphics_fill_mode::none:  draw_shell();   break;
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

} // End of namespace
