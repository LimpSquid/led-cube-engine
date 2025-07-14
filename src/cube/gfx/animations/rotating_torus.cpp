// NB: Created by ChatGPT

#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

struct rotating_torus :
    configurable_animation
{
    rotating_torus(engine_context & context);

    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    int time_ms_;
};

animation_publisher<rotating_torus> const publisher;

constexpr milliseconds default_rotation_time{2000ms};
constexpr double default_radius_cross_section{cube::cube_size_1d / 12.0}; // 12th of the cube size
constexpr double default_radius_main_circle{cube::cube_size_1d / 2.0 - default_radius_cross_section}; // Fill the cube
constexpr double default_hue{360}; // Full hue circle
constexpr int default_iters_main_circle{200};
constexpr int default_iters_cross_section{50};

rotating_torus::rotating_torus(engine_context & context) :
    configurable_animation(context)
{}

void rotating_torus::scene_tick(milliseconds dt)
{
    time_ms_ += static_cast<int>(dt.count());
}

void rotating_torus::paint(graphics_device & device)
{
    painter p(device);

    // Read properties
    auto const rotation_time = read_property<milliseconds>("rotation_time_ms");
    auto const radius_cross_section = read_property<double>("radius_cross_section");
    auto const radius_main_circle = read_property<double>("radius_main_circle");
    auto const iters_cross_section = read_property<int>("iters_cross_section");
    auto const iters_main_circle = read_property<int>("iters_main_circle");
    auto const hue = read_property<double>("hue");

    double omega = (2.0 * M_PI) / static_cast<double>(rotation_time.count());
    double theta = omega * time_ms_; // Spin around vertical axis
    double phi   = 0.5 * omega * time_ms_; // Tilt and pitch

    // Rotation matrices (Z then X):
    double cos_theta = cos(theta), sin_theta = sin(theta);
    double cos_phi = cos(phi), sin_phi = sin(phi);

    // Loop over torus parameters, `u` around main circle, `v` around cross‐section
    for (int i = 0; i < iters_main_circle; ++i) {
        double u = 2.0 * M_PI * i / iters_main_circle;
        double cos_u = cos(u), sin_u = sin(u);

        // Drifting hue and saturation
        double h = hue * i / iters_main_circle + 0.5 * time_ms_ * omega;
        double s = 0.75 + 0.25 * sin(time_ms_ * 0.0005);

        for (int j = 0; j < iters_cross_section; ++j) {
            double v = 2.0 * M_PI * j / iters_cross_section;
            double cos_v = cos(v), sin_v = sin(v);

            // Torus point in local coords
            double x0 = (radius_main_circle + radius_cross_section * cos_v) * cos_u;
            double y0 = (radius_main_circle + radius_cross_section * cos_v) * sin_u;
            double z0 =                       radius_cross_section * sin_v;

            // Apply Z‐rotation (spin)
            double x1 = x0 * cos_theta - y0 * sin_theta;
            double y1 = x0 * sin_theta + y0 * cos_theta;
            double z1 = z0;

            // Apply X‐rotation (tilt)
            double x2 = x1;
            double y2 = y1 * cos_phi - z1 * sin_phi;
            double z2 = y1 * sin_phi + z1 * cos_phi;

            // Convert to voxel coords
            auto xi = int(std::round(x2 + (cube::cube_axis_max_value >> 1)));
            auto yi = int(std::round(y2 + (cube::cube_axis_max_value >> 1)));
            auto zi = int(std::round(z2 + (cube::cube_axis_max_value >> 1)));

            p.set_color(hsv(h, s, 0.75));
            p.draw({ xi, yi, zi });
        }
    }
}

std::unordered_map<std::string, property_value_t> rotating_torus::extra_properties() const
{
    return {
        {"rotation_time_ms", default_rotation_time},
        {"radius_main_circle", default_radius_main_circle},
        {"radius_cross_section", default_radius_cross_section},
        {"iters_main_circle", default_iters_main_circle},
        {"iters_cross_section", default_iters_cross_section},
        {"hue", default_hue}
    };
}

} // End of namespace
