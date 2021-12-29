#include <cube/core/painter.hpp>
#include <cube/core/color.hpp>
#include <cube/core/math.hpp>
#include <3rdparty/glm/geometric.hpp>

namespace cube::core
{

painter::painter(graphics_device & device) :
    device_(device)
{ }

painter::~painter()
{ }

void painter::set_color(color const & color)
{
    state_.draw_color = color;
    state_.dirty_flags |= graphics_state::dirty_draw_color;
}

void painter::draw(voxel_t const & v)
{
    update_state();
    device_.draw(v);
}

void painter::wipe_canvas()
{
    color const old = state_.draw_color;

    set_color(color_black);
    update_state();
    device_.fill();
    set_color(old);
}

void painter::fill_canvas()
{
    update_state();
    device_.fill();
}

void painter::scatter(voxel_t const & origin, double radius, bool smooth)
{
    if (equal(radius, 0.0))
        return draw(origin);

    glm::dvec3 const box{radius, radius, radius};
    glm::dvec3 const min = glm::dvec3(origin) - box;
    glm::dvec3 const max = glm::dvec3(origin) + box;

    int const x_start = std::max(0, static_cast<int>(std::round(min.x)));
    int const y_start = std::max(0, static_cast<int>(std::round(min.y)));
    int const z_start = std::max(0, static_cast<int>(std::round(min.z)));
    int const x_end = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.x)));
    int const y_end = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.y)));
    int const z_end = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.z)));

    color const original_color = state_.draw_color;

    for (int x = x_start; x <= x_end; x++) {
        for (int y = y_start; y <= y_end; y++) {
            for (int z = z_start; z <= z_end; z++) {
                double r = glm::length(glm::dvec3(x, y, z) - glm::dvec3(origin));
                if (less_than_or_equal(r, radius)) {
                    if (smooth) {
                        // 1) cosine to inverse the factor (r / radius), additionally it should
                        //    be a bit smoother than mapping the scalar linearly
                        // 2) we use an offset to only scale the color after (r / radius) reaches
                        //    a certain threshold
                        double scalar = std::min(1.0, 0.4 + std::cos(0.5 * M_PI * (r / radius)));
                        set_color(original_color.vec() * scalar);
                    }

                    draw({x, y, z});
                }
            }
        }
    }

    // reset color
    set_color(original_color);
}

void painter::update_state()
{
    device_.update_state(state_);
    state_.dirty_flags = 0;
}

} // End of namespace
