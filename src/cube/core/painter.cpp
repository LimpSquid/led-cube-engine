#include <cube/core/painter.hpp>
#include <cube/core/color.hpp>
#include <cube/core/parallel.hpp>
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
    // Todo: not nearly as performant as it should be in case radius is large and smoothing is enabled.
    // I think we can improve this by adding a draw box function to the graphics device which can efficiently
    // draw boxes of a single color. That way, for a large radius we can just draw a few boxes around the origin
    // and only draw single pixels at the edges where we are smoothing.
    if (equal(radius, 0.0))
        return draw(origin);

    glm::dvec3 const box{radius, radius, radius};
    glm::dvec3 const min = glm::dvec3(origin) - box;
    glm::dvec3 const max = glm::dvec3(origin) + box;

    int const x_from = std::max(0, static_cast<int>(std::round(min.x)));
    int const y_from = std::max(0, static_cast<int>(std::round(min.y)));
    int const z_from = std::max(0, static_cast<int>(std::round(min.z)));
    int const x_to = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.x))) + 1;
    int const y_to = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.y))) + 1;
    int const z_to = std::min(cube_size_1d - 1, static_cast<int>(std::round(max.z))) + 1;

    update_state(); // In case smooth is false
    color const original_color = state_.draw_color;

    parallel_for({x_from, x_to}, [=](parallel_exclusive_range_t range) {
        for (int x = range.from; x < range.to; x++) {
            for (int y = y_from; y < y_to; y++) {
                for (int z = z_from; z < z_to; z++) {
                    double r = glm::length(glm::dvec3(x, y, z) - glm::dvec3(origin));
                    if (less_than_or_equal(r, radius)) {
                        if (smooth) {
                            // 1) cosine to inverse the factor (r / radius), additionally it should
                            //    be a bit smoother than mapping the scalar linearly
                            // 2) we use an offset to only scale the color after (r / radius) reaches
                            //    a certain threshold
                            double scalar = std::min(1.0, 0.4 + std::cos(0.5 * M_PI * (r / radius)));
                            device_.draw_with_color({x, y, z}, original_color.vec() * scalar);
                        } else
                            device_.draw({x, y, z});
                    }
                }
            }
        }
    });
}

void painter::update_state()
{
    device_.update_state(state_);
    state_.dirty_flags = 0;
}

} // End of namespace
