#include <cube/core/graphics_device.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/math.hpp>
#include <cube/core/logging.hpp>
#include <cube/core/parallel.hpp>
#include <3rdparty/glm/geometric.hpp>

using namespace std::chrono;

namespace cube::core
{

void graphics_device::update_state(graphics_state const & state)
{
    if (state.dirty_flags & graphics_state::dirty_draw_color)
        draw_color_ = state.draw_color;
    if (state.dirty_flags & graphics_state::dirty_fill_mode)
        fill_mode_ = state.fill_mode;
}

void graphics_device::draw(voxel_t const & voxel)
{
    if (visible(voxel))
        blend(draw_color_, buffer_->data[map_to_offset(voxel.x, voxel.y, voxel.z)]);
}

void graphics_device::draw_with_color(voxel_t const & voxel, color const & color)
{
    if (visible(voxel))
        blend(color, buffer_->data[map_to_offset(voxel.x, voxel.y, voxel.z)]);
}

void graphics_device::draw_sphere(voxel_t const & origin, int radius)
{
    radius = std::clamp(radius, 0, cube_size_1d); // clamp to some reasonable values
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
                        blend(draw_color_.vec() * scalar, buffer_->data[offset]);
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
    for (rgba_t & data : *buffer_)
        blend(draw_color_, data);
}

void graphics_device::render(animation & anim)
{
    constexpr int smooth_factor{8};

    if (anim.dirty()) {
        auto const now = steady_clock::now();
        auto const elapsed = std::max(1ms, duration_cast<milliseconds>(now - last_render_tp_));
        auto const render_time = smooth(render_time_acc_, elapsed, smooth_factor);
        LOG_DBG_PERIODIC(10s, "Framerate information.",
            LOG_ARG("frame_time", render_time),
            LOG_ARG("FPS", 1000 / render_time.count()));
        last_render_tp_ = now;

        auto const blur = anim.motion_blur();
        if (blur)
            apply_motion_blur(*blur);

        anim.paint_event(*this);
        show(*buffer_);

        buffer_.flip_and_fill(color_transparent.rgba());
    }
}

graphics_device::graphics_device(engine_context & context) :
    context_(context),
    fill_mode_(graphics_fill_mode::solid),
    draw_color_(color_transparent),
    last_render_tp_(steady_clock::now())
{ }

engine_context & graphics_device::context()
{
    return context_;
}

int graphics_device::map_to_offset(int x, int y, int z) const
{
    return x + y * cube_size_1d + z * cube_size_2d;
}

void graphics_device::apply_motion_blur(double blur)
{
    blur = std::clamp(blur, 0.0, 1.0);

    if constexpr (cube::cube_size_1d >= 32) {
        parallel_for({std::size_t(0), graphics_buffer::size()}, [&](parallel_range_t range) {
            auto prev = buffer_.inactive().begin() + range.from;
            auto data = buffer_->begin() + range.from;
            for (auto i = range.from; i < range.to; ++i)
                blend(map(blur, 0.0, 1.0, color_transparent.vec(), color{*prev++}.vec()), *data++);
        }, use_all_cpus);
    } else {
        auto prev = buffer_.inactive().begin();
        for (rgba_t & data : *buffer_)
            blend(map(blur, 0.0, 1.0, color_transparent.vec(), color{*prev++}.vec()), data);
    }
}

} // End of namespace
